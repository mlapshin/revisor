Revisor
===================

Revisor is an remotely-controlled lightweight browser mainly designed
for integration testing and automating tasks. It's based on
C++/Qt4/WebKit and is cross-platform, fast, and memory efficient
(well, at least it should be). Revisor was designed with minimalistic
approach in mind, so it aims to have less features and small codebase.

Revisor is **NOT** production-ready yet, but who cares? As developer,
you should definitely give him a try. And of course, Revisor is under
active development, so all your patches and suggestions are welcome.

Quick overview
----------

Revisor is a small HTTP server combined with small WebKit renderer. It
listens for HTTP commands and executes them. Commands are hashes
serialized to JSON. Responses are JSON hashes too. You just create a
command, send it to Revisor and get response. Very simple. Obey.

Why Revisor is better?
----------

Or "Why Selenium sucks?" Because:

* No Java Required
* No fight with [Same Origin Policy](http://seleniumhq.org/docs/05_selenium_rc.html#the-same-origin-policy).
* It's lightweight and easy to use. You don't need to run Big Firefox
  with all it's unnecessary GUI and plugins.
* It allows to have completely separate "sessions" within one Revisor
  process. With Selenium you need to start as many Firefox instances,
  as many sessions you need.
* Revisor could be used from almost any environment, because it uses
  HTTP and JSON to communicate with clients.

Compilation
----------

You need Qt 4.6 and ArgTable2 to compile Revisor.

    $ sudo apt-get install libqt4-dev libargtable2-dev
    $ mkdir build && cd build
    $ qmake ../revisor.pro
    $ make

That's all!

Usage
----------

To use Revisor, just run it's executable and start sending requests.

    $ ./revisor -l 127.0.0.1 -p 8080 &
      Revisor is listening "127.0.0.1" on port 8080 PID xxxx
    $ curl --data "command={name:'session.start',session_name:'Hello world!'}" http://127.0.0.1:8080/command

Revisor comes with
[Ruby helper library](http://github.com/sotakone/ruby_revisor_client/),
which you might find useful. This library contains object-oriented
client API, usage examples and test suite.

Command reference
----------

### session.start ###
Starts a new session.

* `session_name` - name to use for created session

### session.stop ###
Stops existing session and closes all tabs associated with it.

* `session_name` - session name to stop

### session.tab.create ###
Creates a tab within a running session.

* `session_name` - existing session name
* `tab_name` - name to use for newly created tab

### session.tab.destroy ###
Destroys an existing tab.

* `session_name` - session name
* `tab_name` - tab name to destroy

### session.tab.visit ###
Open URL in tab.

* `session_name` - existing session name
* `tab_name` - tab to use
* `url` - URL to open

### session.tab.wait\_for\_load ###
Waits for page load. If page is already loaded, returns immediately.

* `session_name` - session name
* `tab_name` - tab name
* `timeout` - wait timeout in msec, default is **0** (no timeout) [optional]

### session.tab.evaluate\_javascript ###
Executes JavaScript in tab context and returns value of last statement.

* `session_name` - session name
* `tab_name` - tab name
* `script` - script source to execute

### session.tab.set\_confirm\_answer ###
Sets answer for JavaScript confirm() function.

* `session_name` - session name
* `tab_name` - tab name
* `answer` - answer in boolean representation (OK - true, Cancel - false)

### session.tab.set\_prompt\_answer ###
Sets answer for JavaScript prompt() function.

* `session_name` - session name
* `tab_name` - tab name
* `answer` - string answer
* `cancelled` - boolean value indicates if 'Cancel' button was pressed

### session.tab.save\_screenshot ###
Saves screenshot of current page to image file.

* `session_name` - session name
* `tab_name` - tab name
* `file_name` - file to render page to
* `viewport_width` - width of resulting image
* `viewport_height` - height of resulting image

### session.set\_cookies ###
Update cookies information or add new ones.

* `session_name` - session name
* `url` - if `domain` or `path` attributes of cookie are not set, they will be extracted from this URL
* `cookies` - array of objects containing information for cookies. Those objects has following keys:
  * `name` - name of the cookie
  * `value` - value of the cookie
  * `path` - URL path where cookie exists
  * `domain` - domain where cookie exists
  * `expires_at` - when cookie expires
  * `http_only` - "HTTP ONLY" flag (?)
  * `secure` - can this cookie be transfered through unencrypted connections

### session.get\_cookies ###
Retrieve list of actual cookies for specified URL.

* `session_name` - session name
* `url` - URL to get cookies for.

### session.tab.send_mouse_event ###
Sends mouse event to a web page in specified tab.

* `session_name` - session name
* `tab_name` - tab name
* `type` - type of event: ['click', 'button_press', 'button_release', 'dblclick', 'move']
* `x` - event x coordinate, relative to page
* `y` - event y coordinate, relative to page
* `button` - button for `click`, `button_press`, `button_release` or `dblclick` events, one of ['left', 'right', 'mid', 'xbutton1', 'xbutton2']
* `buttons` - array of buttons for `move` event
* `modifiers` - array of keyboard modifiers: ['shift', 'control', 'alt', 'meta', 'keypad', 'group_switch']

Copyrights
----------

Copyright (c) 2009 Mikhail Lapshin (sotakone at sotakone dot com),
released under the MIT license.
