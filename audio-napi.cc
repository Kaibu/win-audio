#include <stdio.h>
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <endpointvolume.h>
#include <node_api.h>
#include <psapi.h>
#include <assert.h>
#pragma once

#define VK_MEDIA_NEXT_TRACK 0xB0
#define VK_MEDIA_PLAY_PAUSE 0xB3
#define VK_MEDIA_PREV_TRACK 0xB1
#define VK_MEDIA_STOP 0xB2
#define KEYEVENTF_EXTENDEDKEY 0x0001
#define KEYEVENTF_KEYUP 0x0002
#define SAFE_RELEASE(x) if(x) { x->Release(); x = NULL; }

class VolumeControlNative {
public:
	static float GetMasterVolume() {
		IAudioEndpointVolume *endpoint = GetMaster();
		if (endpoint == NULL)
			return NULL;

		float level;
		endpoint->GetMasterVolumeLevelScalar(&level);

		return level * 100;
	}

	static bool GetMasterMute() {
		IAudioEndpointVolume *endpoint = GetMaster();
		if (endpoint == NULL)
			return NULL;

		BOOL mute;
		endpoint->GetMute(&mute);

		return mute;
	}

	static float GetApplicationVolumePid(int pid)
	{
		ISimpleAudioVolume *volume = GetSessionByPid(pid);
		if (volume == NULL)
			return NULL;

		float level = NULL;
		volume->GetMasterVolume(&level);

		SAFE_RELEASE(volume);

		return level * 100;
	}

	static bool GetApplicationMutePid(int pid)
	{
		ISimpleAudioVolume *volume = GetSessionByPid(pid);
		if (volume == NULL)
			return NULL;

		BOOL *mute = NULL;
		volume->GetMute(mute);

		SAFE_RELEASE(volume);

		return mute;
	}

	static void SetMasterVolume(float level) {
		IAudioEndpointVolume *endpoint = GetMaster();
		if (endpoint == NULL)
			return;

		endpoint->SetMasterVolumeLevelScalar(level / 100, NULL);

		SAFE_RELEASE(endpoint);
	}

	static void SetMasterMute(bool mute) {
		IAudioEndpointVolume *endpoint = GetMaster();
		if (endpoint == NULL)
			return;

		endpoint->SetMute(mute, NULL);

		SAFE_RELEASE(endpoint);
	}

	static void SetApplicationVolumePid(int pid, float level)
	{
		ISimpleAudioVolume *volume = GetSessionByPid(pid);

		if (volume == NULL)
			return;


		volume->SetMasterVolume(level / 100, NULL);

		SAFE_RELEASE(volume);
	}

	static void SetApplicationMutePid(int pid, bool mute)
	{
		ISimpleAudioVolume *volume = GetSessionByPid(pid);
		if (volume == NULL)
			return;

		volume->SetMute(mute, NULL);

		SAFE_RELEASE(volume);
	}

	static void SetApplicationVolumeName(wchar_t* pName, float level)
	{
		ISimpleAudioVolume *volume = GetSessionByName(pName);
		if (volume == NULL)
			return;

		volume->SetMasterVolume(level / 100, NULL);

		SAFE_RELEASE(volume);
	}

	static float GetApplicationVolumeName(wchar_t* pName)
	{
		ISimpleAudioVolume *volume = GetSessionByName(pName);
		if (volume == NULL)
			return NULL;

		float level = NULL;
		volume->GetMasterVolume(&level);

		SAFE_RELEASE(volume);

		return level * 100;
	}

	static bool GetApplicationMuteName(wchar_t* pName)
	{
		ISimpleAudioVolume *volume = GetSessionByName(pName);
		if (volume == NULL)
			return NULL;

		BOOL *mute = NULL;
		volume->GetMute(mute);

		SAFE_RELEASE(volume);

		return mute;
	}

	static void SetApplicationMuteName(wchar_t* pName, bool mute)
	{
		ISimpleAudioVolume *volume = GetSessionByName(pName);
		if (volume == NULL)
			return;

		volume->SetMute(mute, NULL);

		SAFE_RELEASE(volume);
	}

private:
	static IAudioEndpointVolume* GetMaster() {
			int mic = 0;
			HRESULT hr;
			IMMDeviceEnumerator *enumerator = NULL;
			IAudioEndpointVolume *volume = NULL;
			IMMDevice *defaultDevice = NULL;
			CoInitialize(NULL);
			hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *) &enumerator);
			hr = enumerator->GetDefaultAudioEndpoint(mic ? eCapture : eRender, eConsole, &defaultDevice);
			if (hr != 0) {
				return volume;
			}
			hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *) &volume);
			enumerator->Release();
			defaultDevice->Release();
			CoUninitialize();
			return volume;
		}

	static ISimpleAudioVolume* GetSessionByPid(int pid) {
		int mic = 0;
		HRESULT                 hr;
		IMMDeviceEnumerator     *enumerator = NULL;
		ISimpleAudioVolume      *volume = NULL;
		IMMDevice               *device = NULL;
		IAudioSessionManager2   *manager = NULL;
		IAudioSessionEnumerator *sessionEnumerator = NULL;
		int                      sessionCount = 0;

		CoInitialize(NULL);

		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *) &enumerator);

		// Get the default device
		enumerator->GetDefaultAudioEndpoint(mic ? eCapture : eRender, eConsole, &device);

		// Get the session 2 manager
		device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL,
			NULL, (void**)&manager);

		// Get the session enumerator
		manager->GetSessionEnumerator(&sessionEnumerator);

		// Get the session count
		sessionEnumerator->GetCount(&sessionCount);
		// Loop through all sessions
		for (int i = 0; i < sessionCount; i++)
		{
			IAudioSessionControl *ctrl = NULL;
			IAudioSessionControl2 *ctrl2 = NULL;
			DWORD processId = 0;

			hr = sessionEnumerator->GetSession(i, &ctrl);

			if (FAILED(hr))
			{
				continue;
			}

			hr = ctrl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&ctrl2);

			if (FAILED(hr))
			{
				SAFE_RELEASE(ctrl);
				SAFE_RELEASE(ctrl2);
				continue;
			}

			//Identify WMP process
			hr = ctrl2->GetProcessId(&processId);

			if (FAILED(hr))
			{
				SAFE_RELEASE(ctrl);
				SAFE_RELEASE(ctrl2);
				continue;
			}

			if (processId != pid)
			{
				SAFE_RELEASE(ctrl);
				SAFE_RELEASE(ctrl2);
				continue;
			}

			hr = ctrl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&volume);

			if (FAILED(hr))
			{
				SAFE_RELEASE(ctrl);
				SAFE_RELEASE(ctrl2);
				continue;
			}
		}

		return volume;
	}
	static ISimpleAudioVolume* GetSessionByName(wchar_t* pName) {
		int mic = 0;
		HRESULT                 hr;
		IMMDeviceEnumerator     *enumerator = NULL;
		ISimpleAudioVolume      *volume = NULL;
		IMMDevice               *device = NULL;
		IAudioSessionManager2   *manager = NULL;
		IAudioSessionEnumerator *sessionEnumerator = NULL;
		int                      sessionCount = 0;
		CoInitialize(NULL);

		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *) &enumerator);

		// Get the default device
		enumerator->GetDefaultAudioEndpoint(mic ? eCapture : eRender, eConsole, &device);

		// Get the session 2 manager
		device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL,
			NULL, (void**)&manager);

		// Get the session enumerator
		manager->GetSessionEnumerator(&sessionEnumerator);

		// Get the session count
		sessionEnumerator->GetCount(&sessionCount);

		
		// Loop through all sessions
		for (int i = 0; i < sessionCount; i++)
		{
			IAudioSessionControl *ctrl = NULL;
			IAudioSessionControl2 *ctrl2 = NULL;
			DWORD processId = 0;
			wchar_t* displayName = NULL;

			hr = sessionEnumerator->GetSession(i, &ctrl);
		

			if (FAILED(hr))
			{
				continue;
			}

			hr = ctrl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&ctrl2);

			if (FAILED(hr))
			{
				SAFE_RELEASE(ctrl);
				SAFE_RELEASE(ctrl2);
				continue;
			}

			hr = ctrl2->GetSessionIdentifier(&displayName);       


			if (FAILED(hr))
			{
				SAFE_RELEASE(ctrl);
				SAFE_RELEASE(ctrl2);
				continue;
			}

			if (wcsstr(displayName, pName) == 0)
			{
				SAFE_RELEASE(ctrl);
				SAFE_RELEASE(ctrl2);
				continue;
			}

			hr = ctrl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&volume);

			if (FAILED(hr))
			{
				SAFE_RELEASE(ctrl);
				SAFE_RELEASE(ctrl2);
				continue;
			}
		}

		enumerator->Release();
		device->Release();
		CoUninitialize();

		return volume;
	}
};

int *getArgs(napi_env env, napi_callback_info info){
  napi_value argv[2];
  size_t argc = 2;
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  int *out = (int*) malloc(sizeof(int) * argc);
  for(int i = 0; i < (int)argc; i++){
     napi_get_value_int32(env, argv[i], &out[i]);
  }
  return out;
}

wchar_t *getTargetNameArg(napi_env env, napi_callback_info info){ 
  size_t argc = 2;

  napi_value args[2];

  napi_get_cb_info(env, info, &argc, args, NULL, NULL);

  size_t str_size;
  size_t str_size_read;
  napi_get_value_string_utf8(env, args[0], NULL, 0, &str_size);
  str_size = str_size + 1;
  char * buf;
  buf = (char*)calloc(str_size + 1, sizeof(char));
  str_size = str_size + 1;
  napi_get_value_string_utf8(env, args[0], buf, str_size, &str_size_read);

  size_t newSize = strlen(buf) + 1;
  wchar_t * name = new wchar_t[newSize];
  size_t convertedChars = 0;
  mbstowcs_s(&convertedChars, name, newSize, buf, _TRUNCATE);

  return name;
}

napi_value toValue(napi_env env, int value){
  napi_value nvalue = 0;
  napi_create_int32(env, value, &nvalue);
  return nvalue;
}

napi_value getMaster(napi_env env, napi_callback_info info) {

  int *argv = getArgs(env, info);
  float volume = 0;

  volume = VolumeControlNative::GetMasterVolume();

  return toValue(env, (int) round(volume));
}

napi_value setMaster(napi_env env, napi_callback_info info) {

  int *argv = getArgs(env, info);
  float newVolume = ((float)argv[0]);
  
  VolumeControlNative::SetMasterVolume(newVolume);

  return toValue(env, 1);
}

napi_value muteMaster(napi_env env, napi_callback_info info) {

  int *argv = getArgs(env, info);

  VolumeControlNative::SetMasterMute(true);

  return toValue(env, 1);

}

napi_value isMasterMuted(napi_env env, napi_callback_info info) {

  int *argv = getArgs(env, info);

  int mute = 0;
  VolumeControlNative::GetMasterMute() ? mute = 1 : mute = 0;

  return toValue(env, mute);
}

napi_value getApplicationPid(napi_env env, napi_callback_info info) {

  int *argv = getArgs(env, info);
  int pid = ((int)argv[0]);
  float volume = 0;
  
  volume = VolumeControlNative::GetApplicationVolumePid(pid);

  return toValue(env, (int) round(volume));
}

napi_value setApplicationPid(napi_env env, napi_callback_info info) {

  int *argv = getArgs(env, info);
  int pid = ((int)argv[0]);
  float newVolume = ((float)argv[1]);
  
  VolumeControlNative::SetApplicationVolumePid(pid, newVolume);

  float volume = 0;
  volume = VolumeControlNative::GetApplicationVolumePid(pid);

  return toValue(env, (int) round(volume));
}

napi_value setApplicationName(napi_env env, napi_callback_info info) {
  wchar_t * name = getTargetNameArg(env, info);
  int *argv = getArgs(env, info);
  float newVolume = ((float)argv[1]);

  VolumeControlNative::SetApplicationVolumeName(name, newVolume);

  return toValue(env, 1);
}

napi_value getApplicationName(napi_env env, napi_callback_info info) {
  wchar_t * name = getTargetNameArg(env, info);
  int *argv = getArgs(env, info);
  float volume = 0;

  volume = VolumeControlNative::GetApplicationVolumeName(name);

  return toValue(env, (int) round(volume));
}

napi_value setApplicationNameMute(napi_env env, napi_callback_info info) {
  wchar_t * name = getTargetNameArg(env, info);

  VolumeControlNative::SetApplicationMuteName(name, true);

  return toValue(env, 1);
}

napi_value isApplicationNameMuted(napi_env env, napi_callback_info info) {
  wchar_t * name = getTargetNameArg(env, info);

  int mute = 0;
  VolumeControlNative::GetApplicationMuteName(name) ? mute = 1 : mute = 0;

  return toValue(env, mute);
}

napi_value playPause(napi_env env, napi_callback_info info) {

  keybd_event(VK_MEDIA_PLAY_PAUSE, 0, KEYEVENTF_EXTENDEDKEY, 0);
  keybd_event(VK_MEDIA_PLAY_PAUSE, 0, KEYEVENTF_KEYUP, 0);

  return toValue(env, 1);
}

napi_value stop(napi_env env, napi_callback_info info) {

  keybd_event(VK_MEDIA_STOP, 0, KEYEVENTF_EXTENDEDKEY, 0);
  keybd_event(VK_MEDIA_STOP, 0, KEYEVENTF_KEYUP, 0);

  return toValue(env, 1);
}

napi_value next(napi_env env, napi_callback_info info) {

  keybd_event(VK_MEDIA_NEXT_TRACK, 0, KEYEVENTF_EXTENDEDKEY, 0);
  keybd_event(VK_MEDIA_NEXT_TRACK, 0, KEYEVENTF_KEYUP, 0);

  return toValue(env, 1);
}

napi_value previous(napi_env env, napi_callback_info info) {

  keybd_event(VK_MEDIA_PREV_TRACK, 0, KEYEVENTF_EXTENDEDKEY, 0);
  keybd_event(VK_MEDIA_PREV_TRACK, 0, KEYEVENTF_KEYUP, 0);

  return toValue(env, 1);
}


napi_value Init(napi_env env, napi_value exports) {

  napi_status status;
  napi_value get_master_fn, set_master_fn, mute_master_fn, is_master_muted_fn, get_applicationPid_fn, set_applicationPid_fn, set_applicationName_fn, get_applicationName_fn, set_applicationName_mute_fn, is_applicationName_muted_fn, play_pause_fn, stop_fn, next_fn, previous_fn;

  //Master
  status = napi_create_function(env, NULL, 0, getMaster, NULL, &get_master_fn);
  status = napi_create_function(env, NULL, 0, setMaster, NULL, &set_master_fn);
  status = napi_create_function(env, NULL, 0, muteMaster, NULL, &mute_master_fn);
  status = napi_create_function(env, NULL, 0, isMasterMuted, NULL, &is_master_muted_fn);

  //Application
  status = napi_create_function(env, NULL, 0, getApplicationPid, NULL, &get_applicationPid_fn);
  status = napi_create_function(env, NULL, 0, setApplicationPid, NULL, &set_applicationPid_fn);
  status = napi_create_function(env, NULL, 0, setApplicationName, NULL, &set_applicationName_fn);
  status = napi_create_function(env, NULL, 0, getApplicationName, NULL, &get_applicationName_fn);
  status = napi_create_function(env, NULL, 0, setApplicationNameMute, NULL, &set_applicationName_mute_fn);
  status = napi_create_function(env, NULL, 0, isApplicationNameMuted, NULL, &is_applicationName_muted_fn);

  //Media
  status = napi_create_function(env, NULL, 0, playPause, NULL, &play_pause_fn);
  status = napi_create_function(env, NULL, 0, stop, NULL, &stop_fn);
  status = napi_create_function(env, NULL, 0, next, NULL, &next_fn);
  status = napi_create_function(env, NULL, 0, previous, NULL, &previous_fn);

  //Application
  status = napi_set_named_property(env, exports, "getApplicationPid", get_applicationPid_fn);
  status = napi_set_named_property(env, exports, "setApplicationPid", set_applicationPid_fn);
  status = napi_set_named_property(env, exports, "setApplicationName", set_applicationName_fn);
  status = napi_set_named_property(env, exports, "getApplicationName", get_applicationName_fn);
  status = napi_set_named_property(env, exports, "setApplicationNameMute", set_applicationName_mute_fn);
  status = napi_set_named_property(env, exports, "isApplicationNameMuted", is_applicationName_muted_fn);

  //Master
  status = napi_set_named_property(env, exports, "getMaster", get_master_fn);
  status = napi_set_named_property(env, exports, "setMaster", set_master_fn);
  status = napi_set_named_property(env, exports, "muteMaster", mute_master_fn);
  status = napi_set_named_property(env, exports, "isMasterMuted", is_master_muted_fn);

  //Media
  status = napi_set_named_property(env, exports, "playPause", play_pause_fn);
  status = napi_set_named_property(env, exports, "stop", stop_fn);
  status = napi_set_named_property(env, exports, "next", next_fn);
  status = napi_set_named_property(env, exports, "previous", previous_fn);

  assert(status == napi_ok);

  return exports;
}

NAPI_MODULE(addon, Init)