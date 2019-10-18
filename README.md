# win-audio
Get and Set Windows Audio, including audio sessions for process names and pids, also allows you to send media keys like play/pause/skip/previous

not 100% complete and based on the package win-audio

setApplicationName looks up audio sessions for that process, this will work hassle-free for chromium based software like chrome, electron apps etc..

# Install

build for now

### Requirements
[node-gyp](https://github.com/nodejs/node-gyp#installation) to build **audio-napi.cc**

### Version 2.0.0
This version requires **N-API**, and **node** version **>= 8.6.0**

# Module
```javascript
 const win = require('win-audio');

 // manage speaker volume
 const speaker = win.speaker;

 // manage mic volume
 const microphone = win.mic;
```

# Usage

```javascript
const audio = require('win-audio').speaker;

audio.setApplicationName("chrome.exe", value);
```


# Author
Kaibu, based on win-audio by Francesco Saverio Cannizzaro
