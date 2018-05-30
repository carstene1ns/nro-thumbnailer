// nro-thumbnailer main.cpp
// carstene1ns, 2018
// under ISC license, see LICENSE.md

#include <iostream>
#include <fstream>
#include <cstring>
#include <FreeImage.h>
#include "CLI11.hpp"
#include "switch_structs.h"
#include "switch_border.h"

int main(int argc, char **argv) {
	Nro nro;
	NroAsset asset;
	Nacp nacp;

	// options
	std::string in_file;
	std::string out_file;
	unsigned int height = 128, max_skip = 10;
	bool border, extended_info;

	// get xdg dir, fallback to local config
	std::string config_path;
	const char *env = std::getenv("XDG_CONFIG_HOME");
	if (env == nullptr) {
		env = std::getenv("HOME");
		if (env != nullptr)
			config_path = std::string(env) + std::string("/.config");
	} else
		config_path = std::string(env);
	if (!config_path.empty())
		config_path += std::string("/");
	config_path += std::string("nro-thumbnailer.conf");

	// cli parsing
	CLI::App app{"Thumbnailer for Nintendo Switch (homebrew) executables: .nro"};
	app.add_option("-i,--input", in_file, "Input file")->check(CLI::ExistingFile)->group("Files");
	app.add_option("-o,--output", out_file, "Output file")->group("Files");
	app.add_option("-s,--size", height, "Thumbnail image height", true)->check(CLI::Range(16, 1024))->group("Configuration");
	app.add_flag("-b,--border", border, "Add border around thumbnail")->group("Configuration");
	app.add_option("-m,--max-skip", max_skip, "Maximum executable size (in MB)", true)->check(CLI::Range(1, 100))->group("Configuration");
	//app.add_flag("-x,--extended", extended_info, "Add extended information to thumbnail")->group("Configuration");
	app.set_config("--config", config_path, "Configuration file", false);
	CLI11_PARSE(app, argc, argv);

	// sanity
	if (in_file.empty() || out_file.empty()) {
		std::cout << app.help() << std::endl;
		return 1;
	}

	// open file
	std::ifstream nro_file;
	nro_file.open(in_file, std::ios::binary); 
	if (!nro_file.is_open()) {
		std::cout << "Failed to open " << in_file << "!" << std::endl;
		return 1;
	}

	// read in header
	nro_file.read(reinterpret_cast<char *>(&nro), sizeof(Nro));
	if (!nro_file.good()) {
		std::cout << "Could not read header from input file " << in_file << "!" << std::endl;
		nro_file.close();
		return 1;
	}

	// check header
	if (nro.magic != nro_magic) {
		std::cout << "Input file " << in_file << " is not an NRO file (header not found)!" << std::endl;
		nro_file.close();
		return 1;
	}

	// skip, if size is bigger than 10 MB
	if (nro.size > max_skip * 1024 * 1014) {
		std::cout << "Input file " << in_file << " is too big (> " << max_skip << "MB)!" << std::endl;
		nro_file.close();
		return 1;
	}

	nro_file.seekg(nro.size);
	// size >= file size (no asset section)
	if (nro_file.eof()) {
		std::cout << "Input file " << in_file << " has no Asset section!" << std::endl;
		nro_file.close();
		return 1;
	}

	// check asset header
	nro_file.read(reinterpret_cast<char *>(&asset), sizeof(NroAsset));
	if (asset.magic != asset_magic) {
		std::cout << "Input file " << in_file << " has no Asset section (header not found)!" << std::endl;
		nro_file.close();
		return 1;
	}
	// check asset format version
	if (asset.version != asset_version) {
		std::cout << "Input file " << in_file << " has an unknown Asset format version!" << std::endl;
		nro_file.close();
		return 1;
	}

	// no icon included
	if (asset.icon_size == 0) {
		std::cout << "Input file " << in_file << " has no icon!" << std::endl;
		nro_file.close();
		return 1;
	}

	// read the JPEG file
	nro_file.seekg(asset.icon_offset + nro.size);
	char *jpg = new char[asset.icon_size];
	nro_file.read(jpg, asset.icon_size);

	// read nacp, if available
	if (extended_info && asset.nacp_size > 0) {
		nro_file.read(reinterpret_cast<char *>(&nacp), sizeof(Nacp));

		std::cout << "Name: " << nacp.english_name << std::endl
			<< "Author: " << nacp.english_author << std::endl
			<< "Version: " << nacp.version << std::endl;
	}

	nro_file.close();

	// now make a thumbnail
	bool thumb_ok = false;
	FIMEMORY *memjpg, *memborder;
	FIBITMAP *thumb1, *thumb2;
	FreeImage_Initialise(true);

	memjpg = FreeImage_OpenMemory(reinterpret_cast<unsigned char *>(jpg), asset.icon_size);
	if (memjpg != nullptr)
		thumb2 = FreeImage_LoadFromMemory(FIF_JPEG, memjpg, 0);

	if (thumb2 != nullptr) {
		// add border
		if (border) {
			// load background
			memborder = FreeImage_OpenMemory(switch_border, switch_border_size);
			if (memborder != nullptr)
				thumb1 = FreeImage_LoadFromMemory(FIF_PNG, memborder, 0);
			if (thumb1 != nullptr) {
				// copy icon to center
				thumb_ok = FreeImage_Paste(thumb1, thumb2, 115, 8, 256);
			}
			if (thumb_ok) {
				FreeImage_Unload(thumb2);
				thumb2 = thumb1;
			}
		}
	}
	if (thumb2 != nullptr) {
		// scale
		double aspect = static_cast<double>(FreeImage_GetWidth(thumb2)) / FreeImage_GetHeight(thumb2);
		thumb1 = FreeImage_Rescale(thumb2, aspect * height, height, FILTER_BICUBIC);
		if (thumb2 != nullptr)
			thumb_ok = true;
		FreeImage_Unload(thumb2);
	}

	// save the thumbnail
	if (thumb_ok) {
		thumb_ok = FreeImage_Save(FIF_PNG, thumb1, out_file.c_str(), PNG_DEFAULT);
		FreeImage_Unload(thumb1);
	}

	// cleanup
	if (memjpg != nullptr)
		FreeImage_CloseMemory(memjpg);
	if (memborder != nullptr)
		FreeImage_CloseMemory(memborder);
	FreeImage_DeInitialise();

	delete[] jpg;

	// thumbnail creation failed
	if (!thumb_ok) {
		std::cout << "Could not create Thumbnail " << out_file << "!" << std::endl;
		return 1;
	}

	return 0;
}
