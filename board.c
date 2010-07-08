/* board.c
 * 
 * board configuration file.
 */

struct board_description p5100_bd = {
	.name     = "p5100",
	.eth      = "eth2",

	.memsize  = 48*1024*1024,
	.mtdsize  = 16*1024*1024,

	.rtc_file = "/dev/sinfor/rtc",
	.kernel   = "zImage-16sf",
	.kdisk    = "cramfs-16sf.img",
};


struct board_description mig200_bd = {
	.name     = "mig200",
	.eth      = "eth2",

	.memsize  = 32*1024*1024,
	.mtdsize  = 16*1024*1024,

	.rtc_file = "/dev/sinfor/rtc",
	.kernel   = "zImage-16sf",
	.kdisk    = "cramfs-16sf.img",
};

struct board_description secway8695_bd = {
	.name     = "secway8695",
	.eth      = "eth0",

	.memsize  = 48*1024*1024,
	.mtdsize  = 16*1024*1024,

	.rtc_file = "/dev/rtc",
	.kernel   = "",
	.kdisk    = "",
	
};
