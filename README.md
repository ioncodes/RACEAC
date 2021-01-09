# RACEAC
Bypassing EAC integrity checks by abusing a TOCTOU in Dead by Daylight.

## A few words
In an attempt to stop people from cheating by modifying game files, Dead by Daylight received an update that introduced integrity checks for the pak files/assets. In other words, things such as disabling models to get a better view and/or disabling certificate pinning for network interception were no longer possible. Unless...?

## The bug
The bug is quite simple, I stumbled upon this behavior when I was analyzing how DbD loads their assets using Procmon and I noticed that EAC performs checks on the files, but the game itself reopens the file to read the actual content. I am not too familiar how the EAC SDK looks like but I'd assume that the SDK gives you the capability to get a handle to a file (and have it verify it's integrity automatically) and you are supposed to reuse that handle as you read from the file.  
In other words, we found a TOCTOU (Time Of Check, Time Of Use vulnerability)! The game verifies the assets' integrity, but what happens if we modify the assets right before the game reopens the file again, just seconds after finishing the integrity checks? Let's find out...

## The PoC
There's no fancy tricks needed here, all we need is a bit of force when opening files. The PoC disables certificate pinning which allows us to sniff traffic using tools like Fiddler. The SSL settings are stored in the file `pakchunk0-WindowsNoEditor.pak`. Now, let's peek at the code:

```cpp
void detect_eac(std::wstring path)
{
	while (true)
	{
		auto pak = open_pak(path);
		if (pak == INVALID_HANDLE_VALUE) break;
		winapi::handle::close_handle(pak);
		winapi::process::sleep(10);
	}
}

void* race_eac(std::wstring path)
{
	while (true)
	{
		auto pak = open_pak(path);
		if (pak != INVALID_HANDLE_VALUE) return pak;
		winapi::process::sleep(10);
	}

	return nullptr;
}
```

We call `detect_eac` to make sure that the pak file is currently opened and therefore locked by EAC. This leads us to the next stage, `race_eac`. We try to open the pak file until it just works™. Once we get a valid handle the following pattern is replaced with NULL bytes:

```cpp
const std::vector<char> pattern =
{
    0x2B, 0x50, 0x69, 0x6E, 0x6E, 0x65, 0x64, 0x50, 0x75, 0x62, 0x6C, 0x69,
    0x63, 0x4B, 0x65, 0x79, 0x73, 0x3D, 0x22, 0x73, 0x74, 0x65, 0x61, 0x6D,
    0x2E, 0x6C, 0x69, 0x76, 0x65, 0x2E, 0x62, 0x68, 0x76, 0x72, 0x64, 0x62,
    0x64, 0x2E, 0x63, 0x6F, 0x6D, 0x3A, 0x2B, 0x2B, 0x4D, 0x42, 0x67, 0x44,
    0x48, 0x35, 0x57, 0x47, 0x76, 0x4C, 0x39, 0x42, 0x63, 0x6E, 0x35, 0x42,
    0x65, 0x33, 0x30, 0x63, 0x52, 0x63, 0x4C, 0x30, 0x66, 0x35, 0x4F, 0x2B,
    0x4E, 0x79, 0x6F, 0x58, 0x75, 0x57, 0x74, 0x51, 0x64, 0x58, 0x31, 0x61,
    0x49, 0x3D, 0x3B, 0x45, 0x58, 0x72, 0x45, 0x65, 0x2F, 0x58, 0x58, 0x70,
    0x31, 0x6F, 0x34, 0x2F, 0x6E, 0x56, 0x6D, 0x63, 0x71, 0x43, 0x61, 0x47,
    0x2F, 0x42, 0x53, 0x67, 0x56, 0x52, 0x33, 0x4F, 0x7A, 0x68, 0x56, 0x55,
    0x47, 0x38, 0x2F, 0x58, 0x34, 0x6B, 0x52, 0x43, 0x43, 0x55, 0x3D, 0x22
};
```

This pattern translates to the following sequence of chars:

```
+PinnedPublicKeys="steam.live.bhvrdbd.com:++MBgDH5WGvL9Bcn5Be30cRcL0f5O+NyoXuWtQdX1aI=;EXrEe/XXp1o4/nVmcqCaG/BSgVR3OzhVUG8/X4kRCCU="
```

This is one of the public keys used for certificate pinning. Simply removing it from the pak file will disable certificate pinning.

## Demo
The following demo shows a successful bypass, allowing us to dump network traffic. Tools for network analysis can be found on my other [repo](https://github.com/thesecretclub/DeadByDaylight/).

[![](/media/thumbnail.png)](https://streamable.com/w3snm3)

Alternatively, the video is available [here](/media/poc.mp4).

## Last words
The PoC does *not* work with the current version of DbD but it seems like they disabled integrity checks alltogether. That is, the code never breaks out of `race_eac` but restarting DbD without terminating the PoC actually makes it work again. I didn't look at this in the past few months but if you know more, please let me know!