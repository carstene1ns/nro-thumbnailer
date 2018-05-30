// nro-thumbnailer switch_structs.cpp
// carstene1ns, 2018
// under ISC license, see LICENSE.md

// some simplified and minimized helper structures and constants

constexpr int nro_magic = 0x304f524e;   // NRO0
constexpr int asset_magic = 0x54455341; // ASET
constexpr int asset_version = 0;

typedef struct {
	uint8_t skip1[0x10];
	uint32_t magic;
	uint8_t skip2[0x4];
	uint32_t size;
} Nro;

typedef struct {
	uint32_t magic;
	uint32_t version;
	uint64_t icon_offset;
	uint64_t icon_size;
	uint64_t nacp_offset;
	uint64_t nacp_size;
} NroAsset;

typedef struct {
	char english_name[0x200];
	char english_author[0x100];
	uint8_t skip[0x2D60];
	char version[0x10];
} Nacp;
