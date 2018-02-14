#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/gpio.h>
#endif
#include "lcm_drv.h"


#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
/* #include <mach/mt_pm_ldo.h> */
/* #include <mach/mt_gpio.h> */
#endif

#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL, fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif

static LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)			(lcm_util.set_reset_pin((v)))
#define MDELAY(n)					(lcm_util.mdelay(n))

/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
	lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) \
	lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
	lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
	lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
	lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

static unsigned int lcm_compare_id(void);


static const unsigned char LCD_MODULE_ID = 0x01;
/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */
#define LCM_DSI_CMD_MODE								0	/* 1 */
#define FRAME_WIDTH										(720)
#define FRAME_HEIGHT									(1280)

#define REGFLAG_DELAY									0xFC
#define REGFLAG_END_OF_TABLE							0xFD

#ifdef BUILD_LK
#define LCD_BIAS_EN_PIN 				GPIO4
#define LCM_RESET_PIN 					GPIO146

#else

#define LCD_BIAS_EN_PIN				4
#define LCM_RESET_PIN 					146

#endif



#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* --------------------------------------------------------------------------- */
/* Local Variables */
/* --------------------------------------------------------------------------- */

struct LCM_setting_table {
	unsigned char cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_suspend_setting[] = {
	{0x28, 0, {} },
	{0x10, 0, {} },
	{REGFLAG_DELAY, 120, {} }
};

static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x00, 1,{0x00}},
	{0xff, 3,{0x12,0x87,0x01}},
	{0x00, 1,{0x80}},
	{0xff, 2,{0x12,0x87}},
	       
	{0x00, 1,{0x92}},
	{0xff, 2,{0x30,0x02}}, //20
	       
	       
	{0x00, 1,{0x91}},
	{0xb0, 1,{0x92}},
	       
	{0x00, 1,{0x80}},
	{0xc0, 9,{0x00,0x64,0x00,0x0f,0x11,0x00,0x64,0x0f,0x11}},
	       
	{0x00, 1,{0x90}},
	{0xc0, 6,{0x00,0x5c,0x00,0x01,0x00,0x04}},
	       
	{0x00, 1,{0xa4}},
	{0xc0, 1,{0x00}},
	       
	{0x00, 1,{0xb3}},
	{0xc0, 2,{0x00,0x55}},
	       
	{0x00, 1,{0x81}},
	{0xc1, 1,{0x55}},
	       
	       
	{0x00, 1,{0x90}},
	{0xf5, 4,{0x02,0x11,0x02,0x15}},
	       
	{0x00, 1,{0x90}},
	{0xc5, 1,{0x50}},
	       
	{0x00, 1,{0x94}},
	{0xc5, 1,{0x66}},
	       
	       
	{0x00, 1,{0xb2}},
	{0xf5, 2,{0x00,0x00}},
	       
	{0x00, 1,{0xb6}},
	{0xf5, 2,{0x00,0x00}},
	       
	{0x00, 1,{0x94}},
	{0xf5, 2,{0x00,0x00}},
	       
	{0x00, 1,{0xd2}},
	{0xf5, 2,{0x06,0x15}},
	       
	{0x00, 1,{0xb4}},
	{0xc5, 1,{0xcc}},
	       
	       
	{0x00, 1,{0xa0}},
	{0xc4, 14,{0x05,0x10,0x06,0x02,0x05,0x15,0x10,0x05,0x10,0x07,0x02,0x05,0x15,0x10}},
	       
	{0x00, 1,{0xb0}},
	{0xc4, 2,{0x00,0x00}},
	       
	{0x00, 1,{0x91}},
	{0xc5, 2,{0x19,0x52}},
	       
	{0x00, 1,{0x00}},
	{0xd8, 2,{0xbc,0xbc}},
	       
//	{0x00, 1,{0x00}},
//	{0xd9, 1,{0x6d}}, // OT remove 74
	       
	{0x00, 1,{0xb3}},
	{0xc5, 1,{0x84}},
	       
	{0x00, 1,{0xbb}},
	{0xc5, 1,{0x8a}},
	       
	{0x00, 1,{0xb2}},
	{0xc5, 1,{0x40}},
	       
	{0x00, 1,{0x81}},
	{0xc4, 2,{0x82,0x0a}},
	       
	{0x00, 1,{0xc6}},
	{0xB0, 1,{0x03}},
	       
	{0x00, 1,{0xc2}},
	{0xf5, 1,{0x40}},
	       
	{0x00, 1,{0xc3}},
	{0xf5, 1,{0x85}},
	      
	{0x00, 1,{0x00}},
	{0xE1, 20,{0x05,0x3A,0x4A,0x56,0x66,
						0x72,0x74,0x9C,0x89,0xA1,  
						0x64,0x52,0x67,0x47,0x46,
						0x3A,0x2B,0x1E,0x15,0x09}},
	       
	{0x00, 1,{0x00}},
	{0xE2, 20,{0x05,0x3A,0x49,0x56,0x66,
							0x73,0x74,0x9B,0x8A,0xA0,  
							0x65,0x52,0x67,0x47,0x46,
							0x39,0x2B,0x1E,0x15,0x09}},
	       
	       
	{0x00, 1,{0x80}},
	{0xcb, 11,{0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,
						0x00}},
	       
	{0x00, 1,{0x90}},
	{0xcb, 15,{0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00}},
	       
	{0x00, 1,{0xa0}},
	{0xcb, 15,{0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00}},
	       
	{0x00, 1,{0xb0}},
	{0xcb, 15,{0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00}},
	       
	{0x00, 1,{0xc0}},
	{0xcb, 15,{0x05,0x05,0x05,0x05,0x05,
							0x05,0x05,0x05,0x05,0x00,
							0x00,0x00,0x00,0x00,0x00}},
	       
	{0x00, 1,{0xd0}},
	{0xcb, 15,{0x00,0x00,0x00,0x05,0x05,
						0x05,0x05,0x05,0x05,0x05,
						0x05,0x05,0x05,0x05,0x05}},
	       
	{0x00, 1,{0xe0}},
	{0xcb, 14,{0x05,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,
						0x05,0x05,0x05,0x05}},
	       
	{0x00, 1,{0xf0}},
	{0xcb, 11,{0xff,0xff,0xff,0xff,0xff,
						0xff,0xff,0xff,0xff,0xff,0xff}},
	       
	       
	{0x00, 1,{0x80}},
	{0xcc, 15,{0x2d,0x2d,0x0a,0x0c,0x0e,
						0x10,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00}},
	       
	{0x00, 1,{0x90}},
	{0xcc, 15,{0x00,0x00,0x00,0x2e,0x2e,
						0x02,0x04,0x2d,0x2d,0x09,
						0x0b,0x0d,0x0f,0x00,0x00}},
	       
	{0x00, 1,{0xa0}},
	{0xcc, 14,{0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,
						0x2e,0x2e,0x01,0x03}},
	       
	{0x00, 1,{0xb0}},
	{0xcc, 15,{0x2d,0x2e,0x0f,0x0d,0x0b,
						0x09,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00}},
	       
	{0x00, 1,{0xc0}},
	{0xcc, 15,{0x00,0x00,0x00,0x2e,0x2d,
						0x03,0x01,0x2d,0x2e,0x10,
						0x0e,0x0c,0x0a,0x00,0x00}},
	       
	{0x00, 1,{0xd0}},
	{0xcc, 14,{0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,
						0x2e,0x2d,0x04,0x02}},
	       
	       
	{0x00, 1,{0x80}},
	{0xce, 12,{0x8D,0x03,0x29,0x8C,0x03,
						0x29,0x8B,0x03,0x29,0x8A,0x03,0x29}},
	       
	{0x00, 1,{0x90}},
	{0xce, 14,{0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00}},
	      
	{0x00, 1,{0xa0}},
	{0xce, 14,{0x38,0x0B,0x8D,0x00,0x8d,
						0x29,0x00,0x38,0x0A,0x8D,
						0x01,0x8d,0x29,0x00}},
	       
	{0x00, 1,{0xb0}},
	{0xce, 14,{0x38,0x09,0x8D,0x02,0x8d,
						0x29,0x00,0x38,0x08,0x8D,
						0x03,0x8d,0x29,0x00}},
	       
	{0x00, 1,{0xc0}},
	{0xce, 14,{0x38,0x07,0x8D,0x04,0x8d,
						0x29,0x00,0x38,0x06,0x8D,
						0x05,0x8d,0x29,0x00}},
	       
	{0x00, 1,{0xd0}},
	{0xce, 14,{0x38,0x05,0x8D,0x06,0x8d,
						0x29,0x00,0x38,0x04,0x8D,
						0x07,0x8d,0x29,0x00}},
	       
	{0x00, 1,{0x80}},
	{0xcf, 14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	       
	{0x00, 1,{0x90}},
	{0xcf, 14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	       
	{0x00, 1,{0xa0}},
	{0xcf, 14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	       
	{0x00, 1,{0xb0}},
	{0xcf, 14,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	       
	{0x00, 1,{0xc0}},
	{0xcf, 11,{0x01,0x01,0x20,0x20,0x00,
						0x00,0x01,0x01,0x00,0x00,0x00}},
	       
	{0x00, 1,{0xb5}},
	{0xc5, 6,{0x3c,0x01,0xff,0x3c,0x01,0xff}},
	
	//////////////3GAMMA//////
	{0x00,1,{0x00}},
	{0xEC,33,{0x40,0x34,0x44,0x34,0x44,
			0x34,0x44,0x34,0x44,0x34,
			0x44,0x34,0x44,0x34,0x44,
			0x34,0x44,0x34,0x44,0x34,
			0x44,0x34,0x44,0x34,0x44,
			0x34,0x44,0x34,0x44,0x34,
			0x44,0x34,0x04}},

	{0x00,1,{0x00}},
	{0xED,33,{0x40,0x43,0x43,0x43,0x43,
			0x43,0x34,0x34,0x34,0x34,
			0x34,0x44,0x43,0x43,0x43,
			0x43,0x34,0x34,0x34,0x34,
			0x34,0x44,0x43,0x43,0x43,
			0x43,0x43,0x34,0x34,0x34,
			0x34,0x34,0x04}},

	{0x00,1,{0x00}},
	{0xEE,33,{0x40,0x44,0x44,0x44,0x44,
			0x44,0x44,0x44,0x44,0x44,
			0x44,0x44,0x44,0x44,0x44,
			0x44,0x44,0x44,0x44,0x44,
			0x44,0x44,0x44,0x44,0x44,
			0x44,0x44,0x44,0x44,0x44,
			0x44,0x44,0x04}},
	////////////
	       
	{0x00, 1,{0x00}},
	{0xff, 3,{0xff,0xff,0xff}},

	{0x11,  1 ,{0x00}},

	{REGFLAG_DELAY,200,{}},

	{0x29,  1 ,{0x00}},
	
	{REGFLAG_DELAY, 20, {}},
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#ifdef BUILD_LK
static struct LCM_setting_table page1_select[] = {
	//CMD_Page 1
	{0xFF, 3,{0x98,0x81,0x01}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static void push_table(struct LCM_setting_table *table, unsigned int count,
		       unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		unsigned cmd;

		cmd = table[i].cmd;

		switch (cmd) {

		case REGFLAG_DELAY:
			if (table[i].count <= 10)
				MDELAY(table[i].count);
			else
				MDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE:
			break;

		default:
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}

/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
#else
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
#endif

	params->dsi.LANE_NUM = LCM_FOUR_LANE;

	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	params->dsi.packet_size = 256;

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 2;
	params->dsi.vertical_backporch = 16;
	params->dsi.vertical_frontporch = 9;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 20;
	params->dsi.horizontal_backporch = 50;
	params->dsi.horizontal_frontporch = 50;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	/* params->dsi.ssc_disable                                                       = 1; */
#if (LCM_DSI_CMD_MODE)
	params->dsi.PLL_CLOCK = 200;
#else
	params->dsi.PLL_CLOCK = 200;
#endif
}


static void lcm_init(void)
{
#ifdef BUILD_LK
	mt_set_gpio_mode(LCD_BIAS_EN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(LCD_BIAS_EN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(LCD_BIAS_EN_PIN, GPIO_OUT_ONE);	

	mt_set_gpio_mode(LCM_RESET_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(LCM_RESET_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(LCM_RESET_PIN, GPIO_OUT_ONE);
	MDELAY(1);
	
	mt_set_gpio_out(LCM_RESET_PIN, GPIO_OUT_ZERO);
	MDELAY(20);
	
	mt_set_gpio_out(LCM_RESET_PIN, GPIO_OUT_ONE);
	MDELAY(100);
	
#else
	gpio_set_value(LCD_BIAS_EN_PIN, 1);	
	gpio_set_value(LCM_RESET_PIN, 1);
	MDELAY(1);

	gpio_set_value(LCM_RESET_PIN, 0);
	MDELAY(20);

	gpio_set_value(LCM_RESET_PIN, 1);
	MDELAY(100);
#endif

	push_table(lcm_initialization_setting,
		   sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
	push_table(lcm_suspend_setting,
		   sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);

#ifdef BUILD_LK

	mt_set_gpio_mode(LCD_BIAS_EN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(LCD_BIAS_EN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(LCD_BIAS_EN_PIN, GPIO_OUT_ZERO);

	mt_set_gpio_mode(LCM_RESET_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(LCM_RESET_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(LCM_RESET_PIN, GPIO_OUT_ONE);
	MDELAY(10);
	mt_set_gpio_out(LCM_RESET_PIN, GPIO_OUT_ZERO);
	MDELAY(10);

#else
	gpio_set_value(LCM_RESET_PIN, 1);
	MDELAY(10);

	gpio_set_value(LCM_RESET_PIN, 0);
	MDELAY(10);
	
	gpio_set_value(LCD_BIAS_EN_PIN, 0);
#endif
}

static void lcm_resume(void)
{
	lcm_init();
}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = (x0 & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = (x1 & 0xFF);
	unsigned char y0_MSB = ((y0 >> 8) & 0xFF);
	unsigned char y0_LSB = (y0 & 0xFF);
	unsigned char y1_MSB = ((y1 >> 8) & 0xFF);
	unsigned char y1_LSB = (y1 & 0xFF);

	unsigned int data_array[16];

	data_array[0] = 0x00053902;
	data_array[1] = (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00053902;
	data_array[1] = (y1_MSB << 24) | (y0_LSB << 16) | (y0_MSB << 8) | 0x2b;
	data_array[2] = (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}
#endif

#define LCM_ID (0x8712)

static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0;
	
#ifdef BUILD_LK
	unsigned int buffer[5];
	unsigned int array[16];
	
	mt_set_gpio_mode(LCD_BIAS_EN_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(LCD_BIAS_EN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(LCD_BIAS_EN_PIN, GPIO_OUT_ONE);

	mt_set_gpio_mode(LCM_RESET_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(LCM_RESET_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(LCM_RESET_PIN, GPIO_OUT_ONE);
	MDELAY(10);
	
	mt_set_gpio_out(LCM_RESET_PIN, GPIO_OUT_ZERO);
	MDELAY(50);
	
	mt_set_gpio_out(LCM_RESET_PIN, GPIO_OUT_ONE);
	MDELAY(50);

	push_table(page1_select, sizeof(page1_select) / sizeof(struct LCM_setting_table), 1);

	array[0] = 0x00043700;	// read id return 4 byte
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xA1, buffer, 4);
	
    id = (buffer[0]>>16); //use the 4th paramater here
	dprintf(0, "%s, LK otm1287 debug: otm1287 id = 0x%08x\n", __func__, id);
	
#else
	gpio_set_value(LCD_BIAS_EN_PIN, 1);	
	gpio_set_value(LCM_RESET_PIN, 1);
	MDELAY(10);

	gpio_set_value(LCM_RESET_PIN, 0);
	MDELAY(50);

	gpio_set_value(LCM_RESET_PIN, 1);
	MDELAY(50);
	/*****no need to read id in kernel*****/
	printk("%s, Kernel  read otm1287 id but do not thing\n", __func__);
#endif

	if (id == LCM_ID)
		return 1;
	else
		return 0;

}

LCM_DRIVER otm1287_hd720_dsi_vdo_boyi_longwei_lcm_drv = {
	.name = "otm1287_hd720_dsi_vdo_boyi_longwei",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.update = lcm_update,
#endif
};
