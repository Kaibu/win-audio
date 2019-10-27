const EventEmitter = require('events');
const audio = require('../build/Release/audio.node');

var init = (mic) => {

  const events = new EventEmitter();

  var data = {
    audio: audio.getMaster(mic),
    status: audio.isMasterMuted(mic)
  };

  /**
   * Check and update current volume. [Generic]
   */
  var _check = (fn, key, event) => {

    let now = fn(mic);

    if (key == 'status') {
      now = Boolean(now);
    }

    if (data[key] != now) {
      events.emit(event, {
        old: data[key],
        new: now
      });
    }

    data[key] = now;

  };

  /**
   * Check and update current volume.
   */
  var check = () => {
    _check(audio.getMaster, 'audio', 'change');
    _check(audio.isMasterMuted, 'status', 'toggle');
  };

  /**
   * Get current audio
   */
  var getMaster = () => audio.getMaster(mic);

  /**
   * Update current and delegate audio set to native module.
   */
  var setMaster = (value) => {
    audio.setMaster(value, mic);
    check();
  };

  /**
   * Save current status and mute volume.
   */
  var muteMaster = () => audio.muteMaster(mic, 1);


  /**
   * Restore previous volume.
   */
  var unmuteMaster = () => audio.muteMaster(mic, 0);

  /**
   * Mute/Unmute volume.
   */
  var toggle = () => {
    if (audio.isMasterMuted(mic))
      unmuteMaster();
    else
      muteMaster();
  };

  /**
   * React to volume changes using polling check.
   */
  var polling = (interval) => setInterval(check, interval || 500);

  /**
   * Increase current volume of value%
   */
  var increase = (value) => {

    unmuteMaster();

    let perc = data.audio + value;

    if (perc < 0)
      perc = 0;

    if (perc > 100)
      perc = 100;

    setMaster(perc);

  };

  /**
   * Decrease current volume of value%
   */
  var decrease = (value) => increase(-value);


  /**
   * Check if is muted
   */
  var isMasterMuted = () => audio.isMasterMuted(mic) == 1;

  /**
   * play/pause track
   */
  var playPause = () => audio.playPause();

  /**
   * stop track
   */
  var next = () => audio.next();

  /**
   * next track
   */
  var stop = () => audio.stop();

  /**
   * previous track
   */
  var previous = () => audio.previous();

  /**
   * previous track
   */
  var getApplicationPid = (pid) => audio.getApplicationPid(pid);

  /**
   * set volume for pid
   */
  var setApplicationPid = (pid, value) => audio.setApplicationPid(pid, value);

   /**
   * set volume for name
   */
  var setApplicationName = (name, value) => audio.setApplicationName(name, value);

  /**
   * set volume for name
   */
  var getApplicationName = (name) => audio.getApplicationName(name);

  /**
   * mute application by name
   */
  var setApplicationNameUnMute = (name) => audio.setApplicationNameMute(name, 1);


  /**
   * unmute application by name
   */
  var setApplicationNameMute = (name) => audio.setApplicationNameMute(name, 0);

  /**
   * Mute/Unmute volume.
   */
  var toggleApplicationNameMute = () => {
    if (audio.setApplicationNameMute(name))
      setApplicationNameUnMute(name);
    else
      setApplicationNameMute(name);
  };

  return {
    events: events,
    polling: polling,
    getMaster: getMaster,
    setMaster: setMaster,
    increase: increase,
    decrease: decrease,
    getApplicationPid: getApplicationPid,
    setApplicationPid: setApplicationPid,
    setApplicationName: setApplicationName,
    muteMaster: muteMaster,
    unmuteMaster: unmuteMaster,
    isMasterMuted: isMasterMuted,
    toggle: toggle,
    playPause: playPause,
    next: next,
    stop: stop,
    previous: previous
  }

}

module.exports = {
  speaker: init(0),
  mic: init(1)
};
