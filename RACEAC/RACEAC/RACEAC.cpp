#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <io.h>
#include <array>
#include "wrapper.hpp"
#include "slides.hpp"

#define PATCH_SIZE 132

std::vector<char> read_pak(std::wstring pak)
{
	std::vector<char> buffer;
	std::ifstream file(pak, std::ios::in | std::ios::binary);
	file.unsetf(std::ios::skipws);
	file.seekg(0, std::ios::end);
	auto size = file.tellg();
	file.seekg(0, std::ios::beg);
	buffer.reserve(size);
	buffer.insert(
		buffer.begin(),
		std::istream_iterator<char>(file),
		std::istream_iterator<char>());
	return buffer;
}

size_t find_offset(std::vector<char> buffer, std::vector<char> pattern)
{
	auto it = std::search(
		buffer.begin(), buffer.end(),
		pattern.begin(), pattern.end());
	return it - buffer.begin();
}

void patch_offset(void* pak_handle, std::size_t offset, std::vector<char> patch)
{
	auto fd = _open_osfhandle((intptr_t)pak_handle, 0);
	auto file_handle = _fdopen(fd, "w");
	std::ofstream file(file_handle);
	file.seekp(offset, std::ios_base::beg);
	file.write(patch.data(), PATCH_SIZE);
	file.close();
}

void* open_pak(std::wstring pak)
{
	return winapi::file::create_file(
		pak,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);
}

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

int main(int argc, char* argv[])
{
	const auto path = L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Dead by Daylight\\DeadByDaylight\\Content\\Paks\\pakchunk0-WindowsNoEditor.pak";
	const auto size = PATCH_SIZE;
	const auto patch = generate_slide<char>(size, 0x00);
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

	std::cout << "Preparing buffer" << std::endl;
	auto buffer = read_pak(path);
	std::cout << "Looking for offset" << std::endl;
	auto offset = find_offset(buffer, pattern);
	std::cout << "Found offset @ " << std::hex << offset << std::endl;
	detect_eac(path);
	auto pak = race_eac(path);
	std::cout << "Handle => " << pak << std::endl;
	patch_offset(pak, offset, patch);
	std::cout << "Exploited!" << std::endl;
	std::cin.get();
	pak = open_pak(path);
	patch_offset(pak, offset, pattern);
	std::cout << "Exploit unloaded!" << std::endl;
	std::cin.get();
	return 0;
}