#include "dispatcher.hpp"
#include "application.hpp"
#include "session_tab.hpp"
#include "session.hpp"
#include <QDebug>
#include <QScriptValueIterator>
#include <QTime>
#include <QMouseEvent>
#include <QKeyEvent>
#include "exception.hpp"
#include "json.hpp"
#include "main_window.hpp"

#define ARG_FROM_COMMAND(ctype, var, name, type, default)            \
  ctype var = default;                                               \
  if (command.property(name).isValid()) {                            \
    if (!command.property(name).is ## type ()) {                     \
      throw Exception("Parameter " name " should have type " #type); \
    } else {                                                         \
      var = command.property(name).to ## type ();                    \
    }                                                                \
  }

#define STRING_TO_ENUM(svar, evar, s, e) if (svar == s) { evar = e; }

#define COOKIE_PARAM_WITH_CAST(name, pname, type, cast, required)       \
  if (it.value().property(name).is ## type ()) {                        \
    cookie.set ## pname (it.value().property(name).to ## type ().to ## cast ()); \
  } else if (required) {                                                \
    throw Exception("Cookie parameter " name " is required");           \
  }

#define COOKIE_PARAM(name, pname, type, required)                       \
  if (it.value().property(name).is ## type ()) {                        \
    cookie.set ## pname (it.value().property(name).to ## type ());      \
  } else if (required) {                                                \
    throw Exception("Cookie parameter " name " is required");           \
  }


DeferredDispatcherResponseThread::DeferredDispatcherResponseThread(Dispatcher* d)
    : QThread(d), dispatcher(d)
{

}

class WaitForLoadThread : public DeferredDispatcherResponseThread
{
 public:
  WaitForLoadThread(Dispatcher* d, SessionTab* t, int to)
      : DeferredDispatcherResponseThread(d), tab(t), timeout(to) {}

  void run()
  {
    QTime t;
    t.start();
    bool timedOut = !tab->waitForLoad(timeout);
    bool successfull = tab->isLoadSuccessfull();

    response = JSON::response("OK", JSON::keyValue("timed_out", timedOut) + ", " + JSON::keyValue("elapsed_time", t.elapsed()) + ", " + JSON::keyValue("successfull", successfull));
  }

 private:
  SessionTab* tab;
  unsigned int timeout;
};

Dispatcher::Dispatcher(Application* a)
    : QObject(a)
{
  app = a;
}

DispatcherResponse Dispatcher::handleSessionCommand(const QString& commandName, const QScriptValue& command)
{
  DispatcherResponse response;
  assertParamPresent(command, "session_name");
  ARG_FROM_COMMAND(QString, sessionName, "session_name", String, "");

  if (commandName == "session.start") {
    app->startSession(sessionName);
  } else if (commandName == "session.stop") {
    app->stopSession(sessionName);
  } else if (commandName == "session.set_cookies") {
    QList<QNetworkCookie> cookies;
    assertParamPresent(command, "url");
    ARG_FROM_COMMAND(QString, url, "url", String, "");

    if (command.property("cookies").isArray()) {
      QScriptValueIterator it(command.property("cookies"));

      while(it.hasNext()) {
        it.next();

        if (it.name() != "length") {    // ignore length property
          QNetworkCookie cookie;

          COOKIE_PARAM_WITH_CAST("name", Name, String, Utf8, true);
          COOKIE_PARAM_WITH_CAST("value", Value, String, Utf8, true);
          COOKIE_PARAM_WITH_CAST("path", Path, String, Utf8, false);
          COOKIE_PARAM_WITH_CAST("domain", Domain, String, Utf8, false);
          COOKIE_PARAM("http_only", HttpOnly, Bool, false);
          COOKIE_PARAM("secure", Secure, Bool, false);
          if (it.value().property("expires_at").isValid()) {
            QDateTime expiresAt = QDateTime::fromString(it.value().property("expires_at").toString(), Qt::ISODate);
            expiresAt.setTimeSpec(Qt::UTC);
            cookie.setExpirationDate(expiresAt);
          }

          cookies.append(cookie);
        }
      }
    } else {
      throw Exception("Argument 'cookies' of command 'session.set_cookies' must be an array");
    }

    app->getSession(sessionName)->setCookiesFor(cookies, url);
  } else if (commandName == "session.get_cookies") {
    assertParamPresent(command, "url");
    ARG_FROM_COMMAND(QString, url, "url", String, "");

    QList<QNetworkCookie> cookies = app->getSession(sessionName)->getCookiesFor(url);
    response.response = JSON::response("OK", JSON::keyValue("cookies", cookies));
  }

  return response;
}

Qt::MouseButton stringToMouseButton(const QString& button)
{
  Qt::MouseButton realButton = Qt::NoButton;

  STRING_TO_ENUM(button, realButton, "", Qt::NoButton);
  STRING_TO_ENUM(button, realButton, "left", Qt::LeftButton);
  STRING_TO_ENUM(button, realButton, "right", Qt::RightButton);
  STRING_TO_ENUM(button, realButton, "mid", Qt::MidButton);
  STRING_TO_ENUM(button, realButton, "xbutton1", Qt::XButton1);
  STRING_TO_ENUM(button, realButton, "xbutton2", Qt::XButton2);

  return realButton;
}

Qt::KeyboardModifier stringToModifier(const QString& modifier)
{
  Qt::KeyboardModifier ret = Qt::NoModifier;

  STRING_TO_ENUM(modifier, ret, "", Qt::NoModifier);
  STRING_TO_ENUM(modifier, ret, "shift", Qt::ShiftModifier);
  STRING_TO_ENUM(modifier, ret, "control", Qt::ControlModifier);
  STRING_TO_ENUM(modifier, ret, "ctrl", Qt::ControlModifier);
  STRING_TO_ENUM(modifier, ret, "alt", Qt::AltModifier);
  STRING_TO_ENUM(modifier, ret, "meta", Qt::MetaModifier);
  STRING_TO_ENUM(modifier, ret, "keypad", Qt::KeypadModifier);
  STRING_TO_ENUM(modifier, ret, "group_switch", Qt::GroupSwitchModifier);

  return ret;
}

int stringToKey(const QString& key)
{
  int realKey = 0;

  STRING_TO_ENUM(key, realKey, "Escape", Qt::Key_Escape);
  STRING_TO_ENUM(key, realKey, "Tab", Qt::Key_Tab);
  STRING_TO_ENUM(key, realKey, "Backtab", Qt::Key_Backtab);
  STRING_TO_ENUM(key, realKey, "Backspace", Qt::Key_Backspace);
  STRING_TO_ENUM(key, realKey, "Return", Qt::Key_Return);
  STRING_TO_ENUM(key, realKey, "Enter", Qt::Key_Enter);
  STRING_TO_ENUM(key, realKey, "Insert", Qt::Key_Insert);
  STRING_TO_ENUM(key, realKey, "Delete", Qt::Key_Delete);
  STRING_TO_ENUM(key, realKey, "Pause", Qt::Key_Pause);
  STRING_TO_ENUM(key, realKey, "Print", Qt::Key_Print);
  STRING_TO_ENUM(key, realKey, "SysReq", Qt::Key_SysReq);
  STRING_TO_ENUM(key, realKey, "Clear", Qt::Key_Clear);
  STRING_TO_ENUM(key, realKey, "Home", Qt::Key_Home);
  STRING_TO_ENUM(key, realKey, "End", Qt::Key_End);
  STRING_TO_ENUM(key, realKey, "Left", Qt::Key_Left);
  STRING_TO_ENUM(key, realKey, "Up", Qt::Key_Up);
  STRING_TO_ENUM(key, realKey, "Right", Qt::Key_Right);
  STRING_TO_ENUM(key, realKey, "Down", Qt::Key_Down);
  STRING_TO_ENUM(key, realKey, "PageUp", Qt::Key_PageUp);
  STRING_TO_ENUM(key, realKey, "PageDown", Qt::Key_PageDown);
  STRING_TO_ENUM(key, realKey, "Shift", Qt::Key_Shift);
  STRING_TO_ENUM(key, realKey, "Control", Qt::Key_Control);
  STRING_TO_ENUM(key, realKey, "Meta", Qt::Key_Meta);
  STRING_TO_ENUM(key, realKey, "Alt", Qt::Key_Alt);
  STRING_TO_ENUM(key, realKey, "AltGr", Qt::Key_AltGr);
  STRING_TO_ENUM(key, realKey, "CapsLock", Qt::Key_CapsLock);
  STRING_TO_ENUM(key, realKey, "NumLock", Qt::Key_NumLock);
  STRING_TO_ENUM(key, realKey, "ScrollLock", Qt::Key_ScrollLock);
  STRING_TO_ENUM(key, realKey, "F1", Qt::Key_F1);
  STRING_TO_ENUM(key, realKey, "F2", Qt::Key_F2);
  STRING_TO_ENUM(key, realKey, "F3", Qt::Key_F3);
  STRING_TO_ENUM(key, realKey, "F4", Qt::Key_F4);
  STRING_TO_ENUM(key, realKey, "F5", Qt::Key_F5);
  STRING_TO_ENUM(key, realKey, "F6", Qt::Key_F6);
  STRING_TO_ENUM(key, realKey, "F7", Qt::Key_F7);
  STRING_TO_ENUM(key, realKey, "F8", Qt::Key_F8);
  STRING_TO_ENUM(key, realKey, "F9", Qt::Key_F9);
  STRING_TO_ENUM(key, realKey, "F10", Qt::Key_F10);
  STRING_TO_ENUM(key, realKey, "F11", Qt::Key_F11);
  STRING_TO_ENUM(key, realKey, "F12", Qt::Key_F12);
  STRING_TO_ENUM(key, realKey, "F13", Qt::Key_F13);
  STRING_TO_ENUM(key, realKey, "F14", Qt::Key_F14);
  STRING_TO_ENUM(key, realKey, "F15", Qt::Key_F15);
  STRING_TO_ENUM(key, realKey, "F16", Qt::Key_F16);
  STRING_TO_ENUM(key, realKey, "F17", Qt::Key_F17);
  STRING_TO_ENUM(key, realKey, "F18", Qt::Key_F18);
  STRING_TO_ENUM(key, realKey, "F19", Qt::Key_F19);
  STRING_TO_ENUM(key, realKey, "F20", Qt::Key_F20);
  STRING_TO_ENUM(key, realKey, "F21", Qt::Key_F21);
  STRING_TO_ENUM(key, realKey, "F22", Qt::Key_F22);
  STRING_TO_ENUM(key, realKey, "F23", Qt::Key_F23);
  STRING_TO_ENUM(key, realKey, "F24", Qt::Key_F24);
  STRING_TO_ENUM(key, realKey, "F25", Qt::Key_F25);
  STRING_TO_ENUM(key, realKey, "F26", Qt::Key_F26);
  STRING_TO_ENUM(key, realKey, "F27", Qt::Key_F27);
  STRING_TO_ENUM(key, realKey, "F28", Qt::Key_F28);
  STRING_TO_ENUM(key, realKey, "F29", Qt::Key_F29);
  STRING_TO_ENUM(key, realKey, "F30", Qt::Key_F30);
  STRING_TO_ENUM(key, realKey, "F31", Qt::Key_F31);
  STRING_TO_ENUM(key, realKey, "F32", Qt::Key_F32);
  STRING_TO_ENUM(key, realKey, "F33", Qt::Key_F33);
  STRING_TO_ENUM(key, realKey, "F34", Qt::Key_F34);
  STRING_TO_ENUM(key, realKey, "F35", Qt::Key_F35);
  STRING_TO_ENUM(key, realKey, "Super_L", Qt::Key_Super_L);
  STRING_TO_ENUM(key, realKey, "Super_R", Qt::Key_Super_R);
  STRING_TO_ENUM(key, realKey, "Menu", Qt::Key_Menu);
  STRING_TO_ENUM(key, realKey, "Hyper_L", Qt::Key_Hyper_L);
  STRING_TO_ENUM(key, realKey, "Hyper_R", Qt::Key_Hyper_R);
  STRING_TO_ENUM(key, realKey, "Help", Qt::Key_Help);
  STRING_TO_ENUM(key, realKey, "Direction_L", Qt::Key_Direction_L);
  STRING_TO_ENUM(key, realKey, "Direction_R", Qt::Key_Direction_R);
  STRING_TO_ENUM(key, realKey, "Space", Qt::Key_Space);
  STRING_TO_ENUM(key, realKey, "Any", Qt::Key_Any);
  STRING_TO_ENUM(key, realKey, "Exclam", Qt::Key_Exclam);
  STRING_TO_ENUM(key, realKey, "QuoteDbl", Qt::Key_QuoteDbl);
  STRING_TO_ENUM(key, realKey, "NumberSign", Qt::Key_NumberSign);
  STRING_TO_ENUM(key, realKey, "Dollar", Qt::Key_Dollar);
  STRING_TO_ENUM(key, realKey, "Percent", Qt::Key_Percent);
  STRING_TO_ENUM(key, realKey, "Ampersand", Qt::Key_Ampersand);
  STRING_TO_ENUM(key, realKey, "Apostrophe", Qt::Key_Apostrophe);
  STRING_TO_ENUM(key, realKey, "ParenLeft", Qt::Key_ParenLeft);
  STRING_TO_ENUM(key, realKey, "ParenRight", Qt::Key_ParenRight);
  STRING_TO_ENUM(key, realKey, "Asterisk", Qt::Key_Asterisk);
  STRING_TO_ENUM(key, realKey, "Plus", Qt::Key_Plus);
  STRING_TO_ENUM(key, realKey, "Comma", Qt::Key_Comma);
  STRING_TO_ENUM(key, realKey, "Minus", Qt::Key_Minus);
  STRING_TO_ENUM(key, realKey, "Period", Qt::Key_Period);
  STRING_TO_ENUM(key, realKey, "Slash", Qt::Key_Slash);
  STRING_TO_ENUM(key, realKey, "0", Qt::Key_0);
  STRING_TO_ENUM(key, realKey, "1", Qt::Key_1);
  STRING_TO_ENUM(key, realKey, "2", Qt::Key_2);
  STRING_TO_ENUM(key, realKey, "3", Qt::Key_3);
  STRING_TO_ENUM(key, realKey, "4", Qt::Key_4);
  STRING_TO_ENUM(key, realKey, "5", Qt::Key_5);
  STRING_TO_ENUM(key, realKey, "6", Qt::Key_6);
  STRING_TO_ENUM(key, realKey, "7", Qt::Key_7);
  STRING_TO_ENUM(key, realKey, "8", Qt::Key_8);
  STRING_TO_ENUM(key, realKey, "9", Qt::Key_9);
  STRING_TO_ENUM(key, realKey, "Colon", Qt::Key_Colon);
  STRING_TO_ENUM(key, realKey, "Semicolon", Qt::Key_Semicolon);
  STRING_TO_ENUM(key, realKey, "Less", Qt::Key_Less);
  STRING_TO_ENUM(key, realKey, "Equal", Qt::Key_Equal);
  STRING_TO_ENUM(key, realKey, "Greater", Qt::Key_Greater);
  STRING_TO_ENUM(key, realKey, "Question", Qt::Key_Question);
  STRING_TO_ENUM(key, realKey, "At", Qt::Key_At);
  STRING_TO_ENUM(key, realKey, "A", Qt::Key_A);
  STRING_TO_ENUM(key, realKey, "B", Qt::Key_B);
  STRING_TO_ENUM(key, realKey, "C", Qt::Key_C);
  STRING_TO_ENUM(key, realKey, "D", Qt::Key_D);
  STRING_TO_ENUM(key, realKey, "E", Qt::Key_E);
  STRING_TO_ENUM(key, realKey, "F", Qt::Key_F);
  STRING_TO_ENUM(key, realKey, "G", Qt::Key_G);
  STRING_TO_ENUM(key, realKey, "H", Qt::Key_H);
  STRING_TO_ENUM(key, realKey, "I", Qt::Key_I);
  STRING_TO_ENUM(key, realKey, "J", Qt::Key_J);
  STRING_TO_ENUM(key, realKey, "K", Qt::Key_K);
  STRING_TO_ENUM(key, realKey, "L", Qt::Key_L);
  STRING_TO_ENUM(key, realKey, "M", Qt::Key_M);
  STRING_TO_ENUM(key, realKey, "N", Qt::Key_N);
  STRING_TO_ENUM(key, realKey, "O", Qt::Key_O);
  STRING_TO_ENUM(key, realKey, "P", Qt::Key_P);
  STRING_TO_ENUM(key, realKey, "Q", Qt::Key_Q);
  STRING_TO_ENUM(key, realKey, "R", Qt::Key_R);
  STRING_TO_ENUM(key, realKey, "S", Qt::Key_S);
  STRING_TO_ENUM(key, realKey, "T", Qt::Key_T);
  STRING_TO_ENUM(key, realKey, "U", Qt::Key_U);
  STRING_TO_ENUM(key, realKey, "V", Qt::Key_V);
  STRING_TO_ENUM(key, realKey, "W", Qt::Key_W);
  STRING_TO_ENUM(key, realKey, "X", Qt::Key_X);
  STRING_TO_ENUM(key, realKey, "Y", Qt::Key_Y);
  STRING_TO_ENUM(key, realKey, "Z", Qt::Key_Z);
  STRING_TO_ENUM(key, realKey, "BracketLeft", Qt::Key_BracketLeft);
  STRING_TO_ENUM(key, realKey, "Backslash", Qt::Key_Backslash);
  STRING_TO_ENUM(key, realKey, "BracketRight", Qt::Key_BracketRight);
  STRING_TO_ENUM(key, realKey, "AsciiCircum", Qt::Key_AsciiCircum);
  STRING_TO_ENUM(key, realKey, "Underscore", Qt::Key_Underscore);
  STRING_TO_ENUM(key, realKey, "QuoteLeft", Qt::Key_QuoteLeft);
  STRING_TO_ENUM(key, realKey, "BraceLeft", Qt::Key_BraceLeft);
  STRING_TO_ENUM(key, realKey, "Bar", Qt::Key_Bar);
  STRING_TO_ENUM(key, realKey, "BraceRight", Qt::Key_BraceRight);
  STRING_TO_ENUM(key, realKey, "AsciiTilde", Qt::Key_AsciiTilde);
  STRING_TO_ENUM(key, realKey, "nobreakspace", Qt::Key_nobreakspace);
  STRING_TO_ENUM(key, realKey, "exclamdown", Qt::Key_exclamdown);
  STRING_TO_ENUM(key, realKey, "cent", Qt::Key_cent);
  STRING_TO_ENUM(key, realKey, "sterling", Qt::Key_sterling);
  STRING_TO_ENUM(key, realKey, "currency", Qt::Key_currency);
  STRING_TO_ENUM(key, realKey, "yen", Qt::Key_yen);
  STRING_TO_ENUM(key, realKey, "brokenbar", Qt::Key_brokenbar);
  STRING_TO_ENUM(key, realKey, "section", Qt::Key_section);
  STRING_TO_ENUM(key, realKey, "diaeresis", Qt::Key_diaeresis);
  STRING_TO_ENUM(key, realKey, "copyright", Qt::Key_copyright);
  STRING_TO_ENUM(key, realKey, "ordfeminine", Qt::Key_ordfeminine);
  STRING_TO_ENUM(key, realKey, "guillemotleft", Qt::Key_guillemotleft);
  STRING_TO_ENUM(key, realKey, "notsign", Qt::Key_notsign);
  STRING_TO_ENUM(key, realKey, "hyphen", Qt::Key_hyphen);
  STRING_TO_ENUM(key, realKey, "registered", Qt::Key_registered);
  STRING_TO_ENUM(key, realKey, "macron", Qt::Key_macron);
  STRING_TO_ENUM(key, realKey, "degree", Qt::Key_degree);
  STRING_TO_ENUM(key, realKey, "plusminus", Qt::Key_plusminus);
  STRING_TO_ENUM(key, realKey, "twosuperior", Qt::Key_twosuperior);
  STRING_TO_ENUM(key, realKey, "threesuperior", Qt::Key_threesuperior);
  STRING_TO_ENUM(key, realKey, "acute", Qt::Key_acute);
  STRING_TO_ENUM(key, realKey, "mu", Qt::Key_mu);
  STRING_TO_ENUM(key, realKey, "paragraph", Qt::Key_paragraph);
  STRING_TO_ENUM(key, realKey, "periodcentered", Qt::Key_periodcentered);
  STRING_TO_ENUM(key, realKey, "cedilla", Qt::Key_cedilla);
  STRING_TO_ENUM(key, realKey, "onesuperior", Qt::Key_onesuperior);
  STRING_TO_ENUM(key, realKey, "masculine", Qt::Key_masculine);
  STRING_TO_ENUM(key, realKey, "guillemotright", Qt::Key_guillemotright);
  STRING_TO_ENUM(key, realKey, "onequarter", Qt::Key_onequarter);
  STRING_TO_ENUM(key, realKey, "onehalf", Qt::Key_onehalf);
  STRING_TO_ENUM(key, realKey, "threequarters", Qt::Key_threequarters);
  STRING_TO_ENUM(key, realKey, "questiondown", Qt::Key_questiondown);
  STRING_TO_ENUM(key, realKey, "Agrave", Qt::Key_Agrave);
  STRING_TO_ENUM(key, realKey, "Aacute", Qt::Key_Aacute);
  STRING_TO_ENUM(key, realKey, "Acircumflex", Qt::Key_Acircumflex);
  STRING_TO_ENUM(key, realKey, "Atilde", Qt::Key_Atilde);
  STRING_TO_ENUM(key, realKey, "Adiaeresis", Qt::Key_Adiaeresis);
  STRING_TO_ENUM(key, realKey, "Aring", Qt::Key_Aring);
  STRING_TO_ENUM(key, realKey, "AE", Qt::Key_AE);
  STRING_TO_ENUM(key, realKey, "Ccedilla", Qt::Key_Ccedilla);
  STRING_TO_ENUM(key, realKey, "Egrave", Qt::Key_Egrave);
  STRING_TO_ENUM(key, realKey, "Eacute", Qt::Key_Eacute);
  STRING_TO_ENUM(key, realKey, "Ecircumflex", Qt::Key_Ecircumflex);
  STRING_TO_ENUM(key, realKey, "Ediaeresis", Qt::Key_Ediaeresis);
  STRING_TO_ENUM(key, realKey, "Igrave", Qt::Key_Igrave);
  STRING_TO_ENUM(key, realKey, "Iacute", Qt::Key_Iacute);
  STRING_TO_ENUM(key, realKey, "Icircumflex", Qt::Key_Icircumflex);
  STRING_TO_ENUM(key, realKey, "Idiaeresis", Qt::Key_Idiaeresis);
  STRING_TO_ENUM(key, realKey, "ETH", Qt::Key_ETH);
  STRING_TO_ENUM(key, realKey, "Ntilde", Qt::Key_Ntilde);
  STRING_TO_ENUM(key, realKey, "Ograve", Qt::Key_Ograve);
  STRING_TO_ENUM(key, realKey, "Oacute", Qt::Key_Oacute);
  STRING_TO_ENUM(key, realKey, "Ocircumflex", Qt::Key_Ocircumflex);
  STRING_TO_ENUM(key, realKey, "Otilde", Qt::Key_Otilde);
  STRING_TO_ENUM(key, realKey, "Odiaeresis", Qt::Key_Odiaeresis);
  STRING_TO_ENUM(key, realKey, "multiply", Qt::Key_multiply);
  STRING_TO_ENUM(key, realKey, "Ooblique", Qt::Key_Ooblique);
  STRING_TO_ENUM(key, realKey, "Ugrave", Qt::Key_Ugrave);
  STRING_TO_ENUM(key, realKey, "Uacute", Qt::Key_Uacute);
  STRING_TO_ENUM(key, realKey, "Ucircumflex", Qt::Key_Ucircumflex);
  STRING_TO_ENUM(key, realKey, "Udiaeresis", Qt::Key_Udiaeresis);
  STRING_TO_ENUM(key, realKey, "Yacute", Qt::Key_Yacute);
  STRING_TO_ENUM(key, realKey, "THORN", Qt::Key_THORN);
  STRING_TO_ENUM(key, realKey, "ssharp", Qt::Key_ssharp);
  STRING_TO_ENUM(key, realKey, "division", Qt::Key_division);
  STRING_TO_ENUM(key, realKey, "ydiaeresis", Qt::Key_ydiaeresis);
  STRING_TO_ENUM(key, realKey, "Multi_key", Qt::Key_Multi_key);
  STRING_TO_ENUM(key, realKey, "Codeinput", Qt::Key_Codeinput);
  STRING_TO_ENUM(key, realKey, "SingleCandidate", Qt::Key_SingleCandidate);
  STRING_TO_ENUM(key, realKey, "MultipleCandidate", Qt::Key_MultipleCandidate);
  STRING_TO_ENUM(key, realKey, "PreviousCandidate", Qt::Key_PreviousCandidate);
  STRING_TO_ENUM(key, realKey, "Mode_switch", Qt::Key_Mode_switch);
  STRING_TO_ENUM(key, realKey, "Kanji", Qt::Key_Kanji);
  STRING_TO_ENUM(key, realKey, "Muhenkan", Qt::Key_Muhenkan);
  STRING_TO_ENUM(key, realKey, "Henkan", Qt::Key_Henkan);
  STRING_TO_ENUM(key, realKey, "Romaji", Qt::Key_Romaji);
  STRING_TO_ENUM(key, realKey, "Hiragana", Qt::Key_Hiragana);
  STRING_TO_ENUM(key, realKey, "Katakana", Qt::Key_Katakana);
  STRING_TO_ENUM(key, realKey, "Hiragana_Katakana", Qt::Key_Hiragana_Katakana);
  STRING_TO_ENUM(key, realKey, "Zenkaku", Qt::Key_Zenkaku);
  STRING_TO_ENUM(key, realKey, "Hankaku", Qt::Key_Hankaku);
  STRING_TO_ENUM(key, realKey, "Zenkaku_Hankaku", Qt::Key_Zenkaku_Hankaku);
  STRING_TO_ENUM(key, realKey, "Touroku", Qt::Key_Touroku);
  STRING_TO_ENUM(key, realKey, "Massyo", Qt::Key_Massyo);
  STRING_TO_ENUM(key, realKey, "Kana_Lock", Qt::Key_Kana_Lock);
  STRING_TO_ENUM(key, realKey, "Kana_Shift", Qt::Key_Kana_Shift);
  STRING_TO_ENUM(key, realKey, "Eisu_Shift", Qt::Key_Eisu_Shift);
  STRING_TO_ENUM(key, realKey, "Eisu_toggle", Qt::Key_Eisu_toggle);
  STRING_TO_ENUM(key, realKey, "Hangul", Qt::Key_Hangul);
  STRING_TO_ENUM(key, realKey, "Hangul_Start", Qt::Key_Hangul_Start);
  STRING_TO_ENUM(key, realKey, "Hangul_End", Qt::Key_Hangul_End);
  STRING_TO_ENUM(key, realKey, "Hangul_Hanja", Qt::Key_Hangul_Hanja);
  STRING_TO_ENUM(key, realKey, "Hangul_Jamo", Qt::Key_Hangul_Jamo);
  STRING_TO_ENUM(key, realKey, "Hangul_Romaja", Qt::Key_Hangul_Romaja);
  STRING_TO_ENUM(key, realKey, "Hangul_Jeonja", Qt::Key_Hangul_Jeonja);
  STRING_TO_ENUM(key, realKey, "Hangul_Banja", Qt::Key_Hangul_Banja);
  STRING_TO_ENUM(key, realKey, "Hangul_PreHanja", Qt::Key_Hangul_PreHanja);
  STRING_TO_ENUM(key, realKey, "Hangul_PostHanja", Qt::Key_Hangul_PostHanja);
  STRING_TO_ENUM(key, realKey, "Hangul_Special", Qt::Key_Hangul_Special);
  STRING_TO_ENUM(key, realKey, "Dead_Grave", Qt::Key_Dead_Grave);
  STRING_TO_ENUM(key, realKey, "Dead_Acute", Qt::Key_Dead_Acute);
  STRING_TO_ENUM(key, realKey, "Dead_Circumflex", Qt::Key_Dead_Circumflex);
  STRING_TO_ENUM(key, realKey, "Dead_Tilde", Qt::Key_Dead_Tilde);
  STRING_TO_ENUM(key, realKey, "Dead_Macron", Qt::Key_Dead_Macron);
  STRING_TO_ENUM(key, realKey, "Dead_Breve", Qt::Key_Dead_Breve);
  STRING_TO_ENUM(key, realKey, "Dead_Abovedot", Qt::Key_Dead_Abovedot);
  STRING_TO_ENUM(key, realKey, "Dead_Diaeresis", Qt::Key_Dead_Diaeresis);
  STRING_TO_ENUM(key, realKey, "Dead_Abovering", Qt::Key_Dead_Abovering);
  STRING_TO_ENUM(key, realKey, "Dead_Doubleacute", Qt::Key_Dead_Doubleacute);
  STRING_TO_ENUM(key, realKey, "Dead_Caron", Qt::Key_Dead_Caron);
  STRING_TO_ENUM(key, realKey, "Dead_Cedilla", Qt::Key_Dead_Cedilla);
  STRING_TO_ENUM(key, realKey, "Dead_Ogonek", Qt::Key_Dead_Ogonek);
  STRING_TO_ENUM(key, realKey, "Dead_Iota", Qt::Key_Dead_Iota);
  STRING_TO_ENUM(key, realKey, "Dead_Voiced_Sound", Qt::Key_Dead_Voiced_Sound);
  STRING_TO_ENUM(key, realKey, "Dead_Semivoiced_Sound", Qt::Key_Dead_Semivoiced_Sound);
  STRING_TO_ENUM(key, realKey, "Dead_Belowdot", Qt::Key_Dead_Belowdot);
  STRING_TO_ENUM(key, realKey, "Dead_Hook", Qt::Key_Dead_Hook);
  STRING_TO_ENUM(key, realKey, "Dead_Horn", Qt::Key_Dead_Horn);
  STRING_TO_ENUM(key, realKey, "Back", Qt::Key_Back);
  STRING_TO_ENUM(key, realKey, "Forward", Qt::Key_Forward);
  STRING_TO_ENUM(key, realKey, "Stop", Qt::Key_Stop);
  STRING_TO_ENUM(key, realKey, "Refresh", Qt::Key_Refresh);
  STRING_TO_ENUM(key, realKey, "VolumeDown", Qt::Key_VolumeDown);
  STRING_TO_ENUM(key, realKey, "VolumeMute", Qt::Key_VolumeMute);
  STRING_TO_ENUM(key, realKey, "VolumeUp", Qt::Key_VolumeUp);
  STRING_TO_ENUM(key, realKey, "BassBoost", Qt::Key_BassBoost);
  STRING_TO_ENUM(key, realKey, "BassUp", Qt::Key_BassUp);
  STRING_TO_ENUM(key, realKey, "BassDown", Qt::Key_BassDown);
  STRING_TO_ENUM(key, realKey, "TrebleUp", Qt::Key_TrebleUp);
  STRING_TO_ENUM(key, realKey, "TrebleDown", Qt::Key_TrebleDown);
  STRING_TO_ENUM(key, realKey, "MediaPlay", Qt::Key_MediaPlay);
  STRING_TO_ENUM(key, realKey, "MediaStop", Qt::Key_MediaStop);
  STRING_TO_ENUM(key, realKey, "MediaPrevious", Qt::Key_MediaPrevious);
  STRING_TO_ENUM(key, realKey, "MediaNext", Qt::Key_MediaNext);
  STRING_TO_ENUM(key, realKey, "MediaRecord", Qt::Key_MediaRecord);
  STRING_TO_ENUM(key, realKey, "HomePage", Qt::Key_HomePage);
  STRING_TO_ENUM(key, realKey, "Favorites", Qt::Key_Favorites);
  STRING_TO_ENUM(key, realKey, "Search", Qt::Key_Search);
  STRING_TO_ENUM(key, realKey, "Standby", Qt::Key_Standby);
  STRING_TO_ENUM(key, realKey, "OpenUrl", Qt::Key_OpenUrl);
  STRING_TO_ENUM(key, realKey, "LaunchMail", Qt::Key_LaunchMail);
  STRING_TO_ENUM(key, realKey, "LaunchMedia", Qt::Key_LaunchMedia);
  STRING_TO_ENUM(key, realKey, "Launch0", Qt::Key_Launch0);
  STRING_TO_ENUM(key, realKey, "Launch1", Qt::Key_Launch1);
  STRING_TO_ENUM(key, realKey, "Launch2", Qt::Key_Launch2);
  STRING_TO_ENUM(key, realKey, "Launch3", Qt::Key_Launch3);
  STRING_TO_ENUM(key, realKey, "Launch4", Qt::Key_Launch4);
  STRING_TO_ENUM(key, realKey, "Launch5", Qt::Key_Launch5);
  STRING_TO_ENUM(key, realKey, "Launch6", Qt::Key_Launch6);
  STRING_TO_ENUM(key, realKey, "Launch7", Qt::Key_Launch7);
  STRING_TO_ENUM(key, realKey, "Launch8", Qt::Key_Launch8);
  STRING_TO_ENUM(key, realKey, "Launch9", Qt::Key_Launch9);
  STRING_TO_ENUM(key, realKey, "LaunchA", Qt::Key_LaunchA);
  STRING_TO_ENUM(key, realKey, "LaunchB", Qt::Key_LaunchB);
  STRING_TO_ENUM(key, realKey, "LaunchC", Qt::Key_LaunchC);
  STRING_TO_ENUM(key, realKey, "LaunchD", Qt::Key_LaunchD);
  STRING_TO_ENUM(key, realKey, "LaunchE", Qt::Key_LaunchE);
  STRING_TO_ENUM(key, realKey, "LaunchF", Qt::Key_LaunchF);
  STRING_TO_ENUM(key, realKey, "MonBrightnessUp", Qt::Key_MonBrightnessUp);
  STRING_TO_ENUM(key, realKey, "MonBrightnessDown", Qt::Key_MonBrightnessDown);
  STRING_TO_ENUM(key, realKey, "KeyboardLightOnOff", Qt::Key_KeyboardLightOnOff);
  STRING_TO_ENUM(key, realKey, "KeyboardBrightnessUp", Qt::Key_KeyboardBrightnessUp);
  STRING_TO_ENUM(key, realKey, "KeyboardBrightnessDown", Qt::Key_KeyboardBrightnessDown);
  STRING_TO_ENUM(key, realKey, "PowerOff", Qt::Key_PowerOff);
  STRING_TO_ENUM(key, realKey, "WakeUp", Qt::Key_WakeUp);
  STRING_TO_ENUM(key, realKey, "Eject", Qt::Key_Eject);
  STRING_TO_ENUM(key, realKey, "ScreenSaver", Qt::Key_ScreenSaver);
  STRING_TO_ENUM(key, realKey, "WWW", Qt::Key_WWW);
  STRING_TO_ENUM(key, realKey, "Memo", Qt::Key_Memo);
  STRING_TO_ENUM(key, realKey, "LightBulb", Qt::Key_LightBulb);
  STRING_TO_ENUM(key, realKey, "Shop", Qt::Key_Shop);
  STRING_TO_ENUM(key, realKey, "History", Qt::Key_History);
  STRING_TO_ENUM(key, realKey, "AddFavorite", Qt::Key_AddFavorite);
  STRING_TO_ENUM(key, realKey, "HotLinks", Qt::Key_HotLinks);
  STRING_TO_ENUM(key, realKey, "BrightnessAdjust", Qt::Key_BrightnessAdjust);
  STRING_TO_ENUM(key, realKey, "Finance", Qt::Key_Finance);
  STRING_TO_ENUM(key, realKey, "Community", Qt::Key_Community);
  STRING_TO_ENUM(key, realKey, "AudioRewind", Qt::Key_AudioRewind);
  STRING_TO_ENUM(key, realKey, "BackForward", Qt::Key_BackForward);
  STRING_TO_ENUM(key, realKey, "ApplicationLeft", Qt::Key_ApplicationLeft);
  STRING_TO_ENUM(key, realKey, "ApplicationRight", Qt::Key_ApplicationRight);
  STRING_TO_ENUM(key, realKey, "Book", Qt::Key_Book);
  STRING_TO_ENUM(key, realKey, "CD", Qt::Key_CD);
  STRING_TO_ENUM(key, realKey, "Calculator", Qt::Key_Calculator);
  STRING_TO_ENUM(key, realKey, "ToDoList", Qt::Key_ToDoList);
  STRING_TO_ENUM(key, realKey, "ClearGrab", Qt::Key_ClearGrab);
  STRING_TO_ENUM(key, realKey, "Close", Qt::Key_Close);
  STRING_TO_ENUM(key, realKey, "Copy", Qt::Key_Copy);
  STRING_TO_ENUM(key, realKey, "Cut", Qt::Key_Cut);
  STRING_TO_ENUM(key, realKey, "Display", Qt::Key_Display);
  STRING_TO_ENUM(key, realKey, "DOS", Qt::Key_DOS);
  STRING_TO_ENUM(key, realKey, "Documents", Qt::Key_Documents);
  STRING_TO_ENUM(key, realKey, "Excel", Qt::Key_Excel);
  STRING_TO_ENUM(key, realKey, "Explorer", Qt::Key_Explorer);
  STRING_TO_ENUM(key, realKey, "Game", Qt::Key_Game);
  STRING_TO_ENUM(key, realKey, "Go", Qt::Key_Go);
  STRING_TO_ENUM(key, realKey, "iTouch", Qt::Key_iTouch);
  STRING_TO_ENUM(key, realKey, "LogOff", Qt::Key_LogOff);
  STRING_TO_ENUM(key, realKey, "Market", Qt::Key_Market);
  STRING_TO_ENUM(key, realKey, "Meeting", Qt::Key_Meeting);
  STRING_TO_ENUM(key, realKey, "MenuKB", Qt::Key_MenuKB);
  STRING_TO_ENUM(key, realKey, "MenuPB", Qt::Key_MenuPB);
  STRING_TO_ENUM(key, realKey, "MySites", Qt::Key_MySites);
  STRING_TO_ENUM(key, realKey, "News", Qt::Key_News);
  STRING_TO_ENUM(key, realKey, "OfficeHome", Qt::Key_OfficeHome);
  STRING_TO_ENUM(key, realKey, "Option", Qt::Key_Option);
  STRING_TO_ENUM(key, realKey, "Paste", Qt::Key_Paste);
  STRING_TO_ENUM(key, realKey, "Phone", Qt::Key_Phone);
  STRING_TO_ENUM(key, realKey, "Calendar", Qt::Key_Calendar);
  STRING_TO_ENUM(key, realKey, "Reply", Qt::Key_Reply);
  STRING_TO_ENUM(key, realKey, "Reload", Qt::Key_Reload);
  STRING_TO_ENUM(key, realKey, "RotateWindows", Qt::Key_RotateWindows);
  STRING_TO_ENUM(key, realKey, "RotationPB", Qt::Key_RotationPB);
  STRING_TO_ENUM(key, realKey, "RotationKB", Qt::Key_RotationKB);
  STRING_TO_ENUM(key, realKey, "Save", Qt::Key_Save);
  STRING_TO_ENUM(key, realKey, "Send", Qt::Key_Send);
  STRING_TO_ENUM(key, realKey, "Spell", Qt::Key_Spell);
  STRING_TO_ENUM(key, realKey, "SplitScreen", Qt::Key_SplitScreen);
  STRING_TO_ENUM(key, realKey, "Support", Qt::Key_Support);
  STRING_TO_ENUM(key, realKey, "TaskPane", Qt::Key_TaskPane);
  STRING_TO_ENUM(key, realKey, "Terminal", Qt::Key_Terminal);
  STRING_TO_ENUM(key, realKey, "Tools", Qt::Key_Tools);
  STRING_TO_ENUM(key, realKey, "Travel", Qt::Key_Travel);
  STRING_TO_ENUM(key, realKey, "Video", Qt::Key_Video);
  STRING_TO_ENUM(key, realKey, "Word", Qt::Key_Word);
  STRING_TO_ENUM(key, realKey, "Xfer", Qt::Key_Xfer);
  STRING_TO_ENUM(key, realKey, "ZoomIn", Qt::Key_ZoomIn);
  STRING_TO_ENUM(key, realKey, "ZoomOut", Qt::Key_ZoomOut);
  STRING_TO_ENUM(key, realKey, "Away", Qt::Key_Away);
  STRING_TO_ENUM(key, realKey, "Messenger", Qt::Key_Messenger);
  STRING_TO_ENUM(key, realKey, "WebCam", Qt::Key_WebCam);
  STRING_TO_ENUM(key, realKey, "MailForward", Qt::Key_MailForward);
  STRING_TO_ENUM(key, realKey, "Pictures", Qt::Key_Pictures);
  STRING_TO_ENUM(key, realKey, "Music", Qt::Key_Music);
  STRING_TO_ENUM(key, realKey, "Battery", Qt::Key_Battery);
  STRING_TO_ENUM(key, realKey, "Bluetooth", Qt::Key_Bluetooth);
  STRING_TO_ENUM(key, realKey, "WLAN", Qt::Key_WLAN);
  STRING_TO_ENUM(key, realKey, "UWB", Qt::Key_UWB);
  STRING_TO_ENUM(key, realKey, "AudioForward", Qt::Key_AudioForward);
  STRING_TO_ENUM(key, realKey, "AudioRepeat", Qt::Key_AudioRepeat);
  STRING_TO_ENUM(key, realKey, "AudioRandomPlay", Qt::Key_AudioRandomPlay);
  STRING_TO_ENUM(key, realKey, "Subtitle", Qt::Key_Subtitle);
  STRING_TO_ENUM(key, realKey, "AudioCycleTrack", Qt::Key_AudioCycleTrack);
  STRING_TO_ENUM(key, realKey, "Time", Qt::Key_Time);
  STRING_TO_ENUM(key, realKey, "Hibernate", Qt::Key_Hibernate);
  STRING_TO_ENUM(key, realKey, "View", Qt::Key_View);
  STRING_TO_ENUM(key, realKey, "TopMenu", Qt::Key_TopMenu);
  STRING_TO_ENUM(key, realKey, "PowerDown", Qt::Key_PowerDown);
  STRING_TO_ENUM(key, realKey, "Suspend", Qt::Key_Suspend);
  STRING_TO_ENUM(key, realKey, "ContrastAdjust", Qt::Key_ContrastAdjust);
  STRING_TO_ENUM(key, realKey, "MediaLast", Qt::Key_MediaLast);
  STRING_TO_ENUM(key, realKey, "unknown", Qt::Key_unknown);
  STRING_TO_ENUM(key, realKey, "Call", Qt::Key_Call);
  STRING_TO_ENUM(key, realKey, "Context1", Qt::Key_Context1);
  STRING_TO_ENUM(key, realKey, "Context2", Qt::Key_Context2);
  STRING_TO_ENUM(key, realKey, "Context3", Qt::Key_Context3);
  STRING_TO_ENUM(key, realKey, "Context4", Qt::Key_Context4);
  STRING_TO_ENUM(key, realKey, "Flip", Qt::Key_Flip);
  STRING_TO_ENUM(key, realKey, "Hangup", Qt::Key_Hangup);
  STRING_TO_ENUM(key, realKey, "No", Qt::Key_No);
  STRING_TO_ENUM(key, realKey, "Select", Qt::Key_Select);
  STRING_TO_ENUM(key, realKey, "Yes", Qt::Key_Yes);
  STRING_TO_ENUM(key, realKey, "Execute", Qt::Key_Execute);
  STRING_TO_ENUM(key, realKey, "Printer", Qt::Key_Printer);
  STRING_TO_ENUM(key, realKey, "Play", Qt::Key_Play);
  STRING_TO_ENUM(key, realKey, "Sleep", Qt::Key_Sleep);
  STRING_TO_ENUM(key, realKey, "Zoom", Qt::Key_Zoom);
  STRING_TO_ENUM(key, realKey, "Cancel", Qt::Key_Cancel);

  return realKey;
}

Qt::KeyboardModifiers jsonArrayToModifiers(const QScriptValue& array)
{
  Qt::KeyboardModifiers realModifiers;

  if (array.isArray()) {
    int arrayLength = array.property("length").toInt32();

    for (int i = 0; i < arrayLength; i++) {
      QScriptValue v = array.property(i);

      if (v.isString()) {
        realModifiers |= stringToModifier(v.toString());
      }
    }
  }

  return realModifiers;
}

DispatcherResponse Dispatcher::handleSessionTabSendMouseEventCommand(SessionTab* tab, const QScriptValue& command)
{
  DispatcherResponse response;
  QMouseEvent::Type realType;
  Qt::MouseButton realButton;
  Qt::MouseButtons realButtons;
  Qt::KeyboardModifiers realModifiers;

  assertParamPresent(command, "x");
  assertParamPresent(command, "y");
  ARG_FROM_COMMAND(unsigned int, x, "x", Number, 0);
  ARG_FROM_COMMAND(unsigned int, y, "y", Number, 0);

  ARG_FROM_COMMAND(QString, type, "type", String, "click");
  STRING_TO_ENUM(type, realType, "click", QEvent::MouseButtonPress);
  STRING_TO_ENUM(type, realType, "button_press", QEvent::MouseButtonPress);
  STRING_TO_ENUM(type, realType, "button_release", QEvent::MouseButtonRelease);
  STRING_TO_ENUM(type, realType, "dblclick", QEvent::MouseButtonDblClick);
  STRING_TO_ENUM(type, realType, "move", QEvent::MouseMove);

  ARG_FROM_COMMAND(QString, button, "button", String, "");
  realButton = stringToMouseButton(button);

  if ((realType == QEvent::MouseButtonPress ||
       realType == QEvent::MouseButtonRelease ||
       realType == QEvent::MouseButtonDblClick) &&
      realButton == Qt::NoButton) {
    throw Exception(QString("You should specify button for this type of mouse event"));
  }

  QScriptValue buttonsProperty = command.property("buttons");
  if (buttonsProperty.isArray()) {
    int arrayLength = buttonsProperty.property("length").toInt32();

    for (int i = 0; i < arrayLength; i++) {
      QScriptValue v = buttonsProperty.property(i);

      if (v.isString()) {
        realButtons |= stringToMouseButton(v.toString());
      }
    }
  }

  realModifiers = jsonArrayToModifiers(command.property("modifiers"));

  // TODO: this logic should be inside web page or tab, please refactor
  if (type == "click" || type == "dblclick") {
    tab->sendMouseEvent(QEvent::MouseButtonPress, QPoint(x, y), realButton, realButtons, realModifiers);
    tab->sendMouseEvent(QEvent::MouseButtonRelease, QPoint(x, y), realButton, realButtons, realModifiers);

    if (type == "dblclick") {
      tab->sendMouseEvent(QEvent::MouseButtonDblClick, QPoint(x, y), realButton, realButtons, realModifiers);
      tab->sendMouseEvent(QEvent::MouseButtonRelease, QPoint(x, y), realButton, realButtons, realModifiers);
    }
  } else {
    tab->sendMouseEvent(realType, QPoint(x, y), realButton, realButtons, realModifiers);
  }

  return response;
}

DispatcherResponse Dispatcher::handleSessionTabSendKeyEventCommand(SessionTab* tab, const QScriptValue& command)
{
  DispatcherResponse response;
  QMouseEvent::Type realType;
  Qt::Key realKey;
  Qt::KeyboardModifiers realModifiers;

  ARG_FROM_COMMAND(QString, type, "type", String, "press");
  STRING_TO_ENUM(type, realType, "down", QEvent::KeyPress);
  STRING_TO_ENUM(type, realType, "up", QEvent::KeyRelease);

  ARG_FROM_COMMAND(QString, text, "text", String, "");
  ARG_FROM_COMMAND(QString, key, "key", String, "");
  realKey = static_cast<Qt::Key>(stringToKey(key));

  realModifiers = jsonArrayToModifiers(command.property("modifiers"));

  if (type == "press") {
    tab->sendKeyEvent(QEvent::KeyPress, realKey, realModifiers, text, false, 1);
    tab->sendKeyEvent(QEvent::KeyRelease, realKey, realModifiers, text, false, 1);
  } else {
    tab->sendKeyEvent(realType, realKey, realModifiers, text, false, 1);
  }

  return response;
}

DispatcherResponse Dispatcher::handleSessionTabCommand(const QString& commandName, const QScriptValue& command)
{
  assertParamPresent(command, "session_name");
  assertParamPresent(command, "tab_name");

  DispatcherResponse response;
  ARG_FROM_COMMAND(QString, sessionName, "session_name", String, "");
  ARG_FROM_COMMAND(QString, tabName, "tab_name", String, "");
  SessionTab* tab = 0;
  Session* session = 0;

  if (command.property("session_name").isValid() &&
      command.property("tab_name").isValid()) {
    session = app->getSession(sessionName);
    tab = session->getTab(tabName);

    if (tab) {
      app->getMainWindow()->raiseSessionTab(tab);
    }
  }

  if (commandName == "session.tab.create") {
    app->getSession(sessionName)->createTab(tabName);
  } else if (commandName == "session.tab.destroy") {
    app->getSession(sessionName)->destroyTab(tabName);
  } else if (commandName == "session.tab.visit") {
    assertParamPresent(command, "url");
    ARG_FROM_COMMAND(QString, url, "url", String, "");
    tab->visit(url);

  } else if (commandName == "session.tab.wait_for_load") {
    ARG_FROM_COMMAND(unsigned int, timeout, "timeout", Number, 0);

    WaitForLoadThread* t = new WaitForLoadThread(this, tab, timeout);
    t->start();
    response.deferredThread = t;

  } else if (commandName == "session.tab.evaluate_javascript") {
    assertParamPresent(command, "script");
    ARG_FROM_COMMAND(QString, script, "script", String, "");

    QVariant res = tab->evaluateScript(script);
    response.response = JSON::response("OK", JSON::keyValue("eval_result", res));

  } else if (commandName == "session.tab.set_confirm_answer") {
    assertParamPresent(command, "answer");
    ARG_FROM_COMMAND(bool, answer, "answer", Boolean, false);
    tab->setConfirmAnswer(answer);

  } else if (commandName == "session.tab.set_prompt_answer") {
    assertParamPresent(command, "cancelled");
    assertParamPresent(command, "answer");
    ARG_FROM_COMMAND(bool, cancelled, "cancelled", Boolean, false);
    ARG_FROM_COMMAND(QString, answer, "answer", String, "");

    tab->setPromptAnswer(answer, cancelled);

  } else if (commandName == "session.tab.save_screenshot") {
    assertParamPresent(command, "file_name");
    ARG_FROM_COMMAND(QString, fileName, "file_name", String, "");
    ARG_FROM_COMMAND(unsigned int, vpWidth, "viewport_width", Number, 0);
    ARG_FROM_COMMAND(unsigned int, vpHeight, "viewport_height", Number, 0);

    tab->saveScreenshot(fileName, QSize(vpWidth, vpHeight));
  } else if (commandName == "session.tab.send_mouse_event") {
    return handleSessionTabSendMouseEventCommand(tab, command);
  } else if (commandName == "session.tab.send_key_event") {
    return handleSessionTabSendKeyEventCommand(tab, command);
  }

  return response;
}

DispatcherResponse Dispatcher::dispatch(const QScriptValue& command)
{
  DispatcherResponse response;

  try {
    QString commandName = command.property("name").toString();
    QStringList parsedCommandName = commandName.split(".");

    if (parsedCommandName[0] == "session") {
      if (parsedCommandName[1] == "tab") {
        response = handleSessionTabCommand(commandName, command);
      } else {
        response = handleSessionCommand(commandName, command);
      }
    }
  } catch (std::exception& e) {
    response.response = JSON::response("Error", JSON::keyValue("message", QString(e.what())));
  } catch (...) {
    response.response = JSON::response("Error", JSON::keyValue("message", "Unknown error"));
  }

  if (response.response.isEmpty()) {
    response.response = JSON::response("OK");
  }

  return response;
}

void Dispatcher::assertParamPresent(const QScriptValue& c, const QString& p)
{
  QScriptValue s = c.property(p);

  if (!s.isValid()) {
    throw Exception(QString("Parameter '%1' required").arg(p));
  }
}
