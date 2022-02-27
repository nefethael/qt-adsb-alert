#ifndef ADSB_H
#define ADSB_H

#include <cstdint>

typedef enum
{
    UNIT_FEET,
    UNIT_METERS
} altitude_unit_t;

typedef enum
{
    ALTITUDE_BARO,
    ALTITUDE_GEOM
} altitude_source_t;

typedef enum
{
    AG_INVALID = 0,
    AG_GROUND = 1,
    AG_AIRBORNE = 2,
    AG_UNCERTAIN = 3
} airground_t;

typedef enum
{
    SIL_INVALID, SIL_UNKNOWN, SIL_PER_SAMPLE, SIL_PER_HOUR
} sil_type_t;

typedef enum
{
    CPR_INVALID, CPR_SURFACE, CPR_AIRBORNE, CPR_COARSE
} cpr_type_t;

typedef enum
{
   CPR_NONE, CPR_LOCAL, CPR_GLOBAL
} cpr_local_t;

typedef enum
{
    HEADING_INVALID, // Not set
    HEADING_GROUND_TRACK, // Direction of track over ground, degrees clockwise from true north
    HEADING_TRUE, // Heading, degrees clockwise from true north
    HEADING_MAGNETIC, // Heading, degrees clockwise from magnetic north
    HEADING_MAGNETIC_OR_TRUE, // HEADING_MAGNETIC or HEADING_TRUE depending on the HRD bit in opstatus
    HEADING_TRACK_OR_HEADING // GROUND_TRACK / MAGNETIC / TRUE depending on the TAH bit in opstatus
} heading_type_t;

typedef enum {
    COMMB_UNKNOWN,
    COMMB_AMBIGUOUS,
    COMMB_EMPTY_RESPONSE,
    COMMB_DATALINK_CAPS,
    COMMB_GICB_CAPS,
    COMMB_AIRCRAFT_IDENT,
    COMMB_ACAS_RA,
    COMMB_VERTICAL_INTENT,
    COMMB_TRACK_TURN,
    COMMB_HEADING_SPEED
} commb_format_t;

typedef enum
{
    NAV_MODE_AUTOPILOT = 1,
    NAV_MODE_VNAV = 2,
    NAV_MODE_ALT_HOLD = 4,
    NAV_MODE_APPROACH = 8,
    NAV_MODE_LNAV = 16,
    NAV_MODE_TCAS = 32
} nav_modes_t;

// Matches encoding of the ES type 28/1 emergency/priority status subfield

typedef enum
{
    EMERGENCY_NONE = 0,
    EMERGENCY_GENERAL = 1,
    EMERGENCY_LIFEGUARD = 2,
    EMERGENCY_MINFUEL = 3,
    EMERGENCY_NORDO = 4,
    EMERGENCY_UNLAWFUL = 5,
    EMERGENCY_DOWNED = 6,
    EMERGENCY_RESERVED = 7
} emergency_t;

typedef enum {
    NAV_ALT_INVALID,
    NAV_ALT_UNKNOWN,
    NAV_ALT_AIRCRAFT,
    NAV_ALT_MCP,
    NAV_ALT_FMS
} nav_altitude_source_t;

typedef enum
{
    ADDR_ADSB_ICAO = 0, /* ADS-B, ICAO address, transponder sourced */
    ADDR_ADSB_ICAO_NT = 1, /* ADS-B, ICAO address, non-transponder */
    ADDR_ADSR_ICAO = 2, /* ADS-R, ICAO address */
    ADDR_TISB_ICAO = 3, /* TIS-B, ICAO address */

    ADDR_JAERO = 4,
    ADDR_MLAT = 5,
    ADDR_OTHER = 6,
    ADDR_MODE_S = 7,

    ADDR_ADSB_OTHER = 8, /* ADS-B, other address format */
    ADDR_ADSR_OTHER = 9, /* ADS-R, other address format */
    ADDR_TISB_TRACKFILE = 10, /* TIS-B, Mode A code + track file number */
    ADDR_TISB_OTHER = 11, /* TIS-B, other address format */

    ADDR_MODE_A = 12, /* Mode A */

    ADDR_UNKNOWN = 15/* unknown address format */
} addrtype_t;

#pragma pack(push,1)
struct binHeader {
    int64_t	time;
    uint32_t elementSize;
    uint32_t ac_count_pos;
    uint32_t globeIndex;
    int16_t	south;
    int16_t	west;
    int16_t	north;
    int16_t	east;
    uint32_t messageCount;
};
struct binCraft {
  uint32_t hex;
  uint16_t seen_pos;
  uint16_t seen;
  // 8
  int32_t lon;
  int32_t lat;
  // 16
  int16_t baro_rate;
  int16_t geom_rate;
  int16_t baro_alt;
  int16_t geom_alt;
  // 24
  uint16_t nav_altitude_mcp; // FCU/MCP selected altitude
  uint16_t nav_altitude_fms; // FMS selected altitude
  int16_t nav_qnh; // Altimeter setting (QNH/QFE), millibars
  int16_t nav_heading; // target heading, degrees (0-359)
  // 32
  uint16_t squawk; // Squawk
  int16_t gs;
  int16_t mach;
  int16_t roll; // Roll angle, degrees right
  // 40
  int16_t track; // Ground track
  int16_t track_rate; // Rate of change of ground track, degrees/second
  int16_t mag_heading; // Magnetic heading
  int16_t true_heading; // True heading
  // 48
  int16_t wind_direction;
  int16_t wind_speed;
  int16_t oat;
  int16_t tat;
  // 56
  uint16_t tas;
  uint16_t ias;
  uint16_t pos_rc; // Rc of last computed position
  uint16_t messages;

  // 64
  char category;
  //uint32_t category:8; // Aircraft category A0 - D7 encoded as a single hex byte. 00 = unset

  char pos_nic;
  //uint32_t pos_nic:8; // NIC of last computed position

  // 66
  char nav_modes;
  //uint32_t nav_modes:8; // enabled modes (autopilot, vnav, etc)

  char pad67;
  //uint32_t emergency:4; // Emergency/priority status
  //uint32_t addrtype:4; // highest priority address type seen for this aircraft

  // 68
  char pad68;
  //uint32_t airground:4; // air/ground status
  //uint32_t nav_altitude_src:4;  // source of altitude used by automation

  char pad69;
  //uint32_t sil_type:4; // SIL supplement from TSS or opstatus
  //uint32_t adsb_version:4; // ADS-B version (from ADS-B operational status); -1 means no ADS-B messages seen

  // 70
  char pad70;
  //uint32_t adsr_version:4; // As above, for ADS-R messages
  //uint32_t tisb_version:4; // As above, for TIS-B messages

  char pad71;
  //uint32_t nac_p : 4; // NACp from TSS or opstatus
  //uint32_t nac_v : 4; // NACv from airborne velocity or opstatus

  // 72
  char pad72;
  // uint32_t sil : 2; // SIL from TSS or opstatus
  // uint32_t gva : 2; // GVA from opstatus
  // uint32_t sda : 2; // SDA from opstatus
  // uint32_t nic_a : 1; // NIC supplement A from opstatus
  // uint32_t nic_c : 1; // NIC supplement C from opstatus

  char pad73;
  // uint32_t nic_baro : 1; // NIC baro supplement from TSS or opstatus
  // uint32_t alert : 1; // FS Flight status alert bit
  // uint32_t spi : 1; // FS Flight status SPI (Special Position Identification) bit
  // uint32_t callsign_valid:1;
  // uint32_t baro_alt_valid:1;
  // uint32_t geom_alt_valid:1;
  // uint32_t position_valid:1;
  // uint32_t gs_valid:1;

  // 74
  char pad74;
  // uint32_t ias_valid:1;
  // uint32_t tas_valid:1;
  // uint32_t mach_valid:1;
  // uint32_t track_valid:1;
  // uint32_t track_rate_valid:1;
  // uint32_t roll_valid:1;
  // uint32_t mag_heading_valid:1;
  // uint32_t true_heading_valid:1;

  char pad75;
  //uint32_t baro_rate_valid:1;
  //uint32_t geom_rate_valid:1;
  //uint32_t nic_a_valid:1;
  //uint32_t nic_c_valid:1;
  //uint32_t nic_baro_valid:1;
  //uint32_t nac_p_valid:1;
  //uint32_t nac_v_valid:1;
  //uint32_t sil_valid:1;

  // 76
  char pad76;
  // uint32_t gva_valid:1;
  // uint32_t sda_valid:1;
  // uint32_t squawk_valid:1;
  // uint32_t emergency_valid:1;
  // uint32_t spi_valid:1;
  // uint32_t nav_qnh_valid:1;
  // uint32_t nav_altitude_mcp_valid:1;
  // uint32_t nav_altitude_fms_valid:1;

  char pad77;
  // uint32_t nav_altitude_src_valid:1;
  // uint32_t nav_heading_valid:1;
  // uint32_t nav_modes_valid:1;
  // uint32_t alert_valid:1;
  // uint32_t wind_valid:1;
  // uint32_t temp_valid:1;
  // uint32_t unused_1:1;
  // uint32_t unused_2:1;

  // 78
  char callsign[8]; // Flight number
  // 86
  uint16_t dbFlags;
  // 88
  char typeCode[4];
  // 92
  char registration[12];
  // 104
  uint8_t receiverCount;
  uint8_t signal;
  uint8_t extraFlags;
  uint8_t reserved;
  // 108
};
#pragma pack(pop)


#endif // ADSB_H
