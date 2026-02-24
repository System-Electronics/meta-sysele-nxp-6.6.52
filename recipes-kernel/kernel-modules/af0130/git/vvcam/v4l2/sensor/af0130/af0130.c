/*
* Copyright (C) 2012-2015 Freescale Semiconductor, Inc. All Rights Reserved.
* Copyright 2018 NXP
* Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
* Copyright 2022 Semiconductor Components Industries, LLC ("onsemi").
*/
/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/of_graph.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/v4l2-mediabus.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-fwnode.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include "vvsensor.h"
#include "af0130_regs_1M.h"
#include "af0130_regs_480p.h"

#define AF0130_VOLTAGE_ANALOG			2800000
#define AF0130_VOLTAGE_DIGITAL_CORE		1200000
#define AF0130_VOLTAGE_DIGITAL_IO		1800000

#define AF0130_XCLK_MIN 6000000
#define AF0130_XCLK_MAX 48000000

#define AF0130_CHIP_ID                  0x0657
#define AF0130_CHIP_VERSION_REG 		0x0016

#define AF0130_SENS_PAD_SOURCE	0
#define AF0130_SENS_PADS_NUM	1

#define AF0130_REG_CTRL_MODE		0x0100
#define AF0130_MODE_SW_STANDBY	0x00
#define AF0130_MODE_STREAMING		0x01

#define AF0130_GAIN_MIN		0x00
#define AF0130_GAIN_MAX		127
#define AF0130_REG_GAIN		0x0208
#define AF0130_REG_GAIN_FINE	0x0206

#define client_to_af0130(client)\
	container_of(i2c_get_clientdata(client), struct af0130, subdev)

struct af0130_capture_properties {
	__u64 max_lane_frequency;
	__u64 max_pixel_frequency;
	__u64 max_data_rate;
};
/*declare for eeprom started.*/
struct camera_common_eeprom_data {
        struct i2c_client *i2c_client;
        struct i2c_adapter *adap;
        struct i2c_board_info brd;
};

char* pga_labels[]={
	"100MHz",
	"100_120M",
	"120MHz",
	"175MHz",
	"175_200M",
	"200MHz",
	"30MHz",
	"30_35MHz",
	"35MHz",
	"75MHz",
};
#define MAX_SUB_MODE_NUM 5
typedef struct{
	char* mode_name;
	unsigned short list_len;
	unsigned short total_size;
	char* sub_name[MAX_SUB_MODE_NUM];
} mode_list;
mode_list eeprom_mode_settings[]={
	{"100M",     1, 532,  {"100M"}},
	{"175_200M", 3, 1064, {"175M", "F1_200M", "175_200M"}},
};
#define AF0130_EEPROM_ADDRESS 0x54
#define PGA_TYPE 0x08
#define PGA_HEAD_SIZE 11
#define PGA_START_ADDR 0x0F
#define MAX_EEPROM_BUF_SIZE 1071
#define TL_EEPROM_CMN_AREA_SIZE_ (0x08A0U)
static u8 eeprom_buf[MAX_EEPROM_BUF_SIZE] = {0};
static u8 used_freq_num = 1;
/*declare for eeprom ended.*/
struct af0130 {
	struct i2c_client *i2c_client;
	struct regulator *io_regulator;
	struct regulator *core_regulator;
	struct regulator *analog_regulator;
	unsigned int pwn_gpio;
	unsigned int rst_gpio;
	unsigned int mclk;
	unsigned int mclk_source;
	struct clk *sensor_clk;
	unsigned int csi_id;
	struct af0130_capture_properties ocp;

	struct v4l2_subdev subdev;
	struct media_pad pads[AF0130_SENS_PADS_NUM];

	struct v4l2_mbus_framefmt format;
	vvcam_mode_info_t cur_mode;
	sensor_blc_t blc;
	sensor_white_balance_t wb;
	struct mutex lock;
	u32 stream_status;
	u32 resume_status;
	vvcam_lens_t focus_lens;
	bool has_eeprom;
	bool eeprom_data_ready;
	u16 data_len;
	struct camera_common_eeprom_data eeprom;
};
struct gain_table{
	unsigned int code;
	unsigned int times;
};

unsigned int GAIN_TABLE_SIZE = 128;
struct gain_table af0130_gain_table[] = {
{0	,1024     },
{1	,1069     },
{2	,1116     },
{3	,1166     },
{4	,1217     },
{5	,1271     },
{6	,1327     },
{7	,1385     },
{8	,1441     },
{9	,1505     },
{10	,1571     },
{11	,1641     },
{12	,1713     },
{13	,1789     },
{14	,1868     },
{15	,1950     },
{16	,2038     },
{17	,2127     },
{18	,2221     },
{19	,2319     },
{20	,2422     },
{21	,2529     },
{22	,2640     },
{23	,2757     },
{24	,2880     },
{25	,3007     },
{26	,3140     },
{27	,3278     },
{28	,3423     },
{29	,3574     },
{30	,3732     },
{31	,3896     },
{32	,4064     },
{33	,4243     },
{34	,4430     },
{35	,4626     },
{36	,4830     },
{37	,5043     },
{38	,5265     },
{39	,5498     },
{40	,5758     },
{41	,6012     },
{42	,6277     },
{43	,6554     },
{44	,6843     },
{45	,7145     },
{46	,7460     },
{47	,7789     },
{48	,8109     },
{49	,8467     },
{50	,8840     },
{51	,9230     },
{52	,9638     },
{53	,10063    },
{54	,10507    },
{55	,10970    },
{56	,11472    },
{57	,11978    },
{58	,12507    },
{59	,13059    },
{60	,13635    },
{61	,14236    },
{62	,14864    },
{63	,15520    },
{64	,16182    },
{65	,16896    },
{66	,17642    },
{67	,18420    },
{68	,19233    },
{69	,20081    },
{70	,20967    },
{71	,21892    },
{72	,22861    },
{73	,23870    },
{74	,24923    },
{75	,26022    },
{76	,27170    },
{77	,28369    },
{78	,29621    },
{79	,30927    },
{80	,32357    },
{81	,33785    },
{82	,35275    },
{83	,36831    },
{84	,38456    },
{85	,40153    },
{86	,41925    },
{87	,43774    },
{88	,45722    },
{89	,47739    },
{90	,49845    },
{91	,52044    },
{92	,54341    },
{93	,56738    },
{94	,59241    },
{95	,61855    },
{96	,64538    },
{97	,67386    },
{98	,70359    },
{99	,73463    },
{100,	76704 },
{101,	80088 },
{102,	83621 },
{103,	87311 },
{104,	91180 },
{105,	95203 },
{106,	99403 },
{107,	103789},
{108,	108368},
{109,	113149},
{110,	118141},
{111,	123353},
{112,	128901},
{113,	134588},
{114,	140525},
{115,	146725},
{116,	153199},
{117,	159958},
{118,	167015},
{119,	174383},
{120,	182008},
{121,	190038},
{122,	198423},
{123,	207177},
{124,	216317},
{125,	225861},
{126,	235826},
{127,	246230},
};

static struct vvcam_mode_info_s paf0130_mode_info[] = {
	{
		.index          = 0,
		.size           = {
			.bounds_width  = 640,
			.bounds_height = 1440,
			.top           = 0,
			.left          = 0,
			.width         = 640,
			.height        = 1440,//480*3
		},
		.hdr_mode       = SENSOR_MODE_LINEAR,
		.bit_width      = 12,
		.data_compress  = {
			.enable = 0,
		},
		.bayer_pattern = BAYER_Y12,
		.ae_info = {
			.def_frm_len_lines     = 0x1C92,
			.curr_frm_len_lines    = 0x1C92 - 1,
			.one_line_exp_time_ns  = 4540,

			.max_integration_line  = 131,//0x8A8 - 1,
			.min_integration_line  = 8,

			.max_again             = 240.45 * (1 << SENSOR_FIX_FRACBITS),
			.min_again             = 1 * (1 << SENSOR_FIX_FRACBITS),
			.max_dgain             = 1 * (1 << SENSOR_FIX_FRACBITS),
			.min_dgain             = 1 * (1 << SENSOR_FIX_FRACBITS),
			.gain_step             = 1,

			.start_exposure        = 3 * 100 * (1 << SENSOR_FIX_FRACBITS),
			.cur_fps               = 30 * (1 << SENSOR_FIX_FRACBITS),
			.max_fps               = 30 * (1 << SENSOR_FIX_FRACBITS),
			.min_fps               = 5 * (1 << SENSOR_FIX_FRACBITS),
			.min_afps              = 5 * (1 << SENSOR_FIX_FRACBITS),
			.int_update_delay_frm  = 1,
			.gain_update_delay_frm = 1,
		},
		.mipi_info = {
			.mipi_lane = 2,
		},
		.preg_data      = af0130_init_setting_linear_480p,
		.reg_data_count = ARRAY_SIZE(af0130_init_setting_linear_480p),
	},
	{
		.index          = 1,
		.size           = {
			.bounds_width  = 1280,
			.bounds_height = 2880,
			.top           = 0,
			.left          = 0,
			.width         = 1280,
			.height        = 2880,//960*3+2
		},
		.hdr_mode       = SENSOR_MODE_LINEAR,
		.bit_width      = 12,
		.data_compress  = {
			.enable = 0,
		},
		.bayer_pattern = BAYER_Y12,
		.ae_info = {
			.def_frm_len_lines     = 0x1649,
			.curr_frm_len_lines    = 0x1649 - 1,
			.one_line_exp_time_ns  = 5842,

			.max_integration_line  = 131,//0x8A8 - 1,
			.min_integration_line  = 8,

			.max_again             = 240.45 * (1 << SENSOR_FIX_FRACBITS),
			.min_again             = 1 * (1 << SENSOR_FIX_FRACBITS),
			.max_dgain             = 1 * (1 << SENSOR_FIX_FRACBITS),
			.min_dgain             = 1 * (1 << SENSOR_FIX_FRACBITS),
			.gain_step             = 1,

			.start_exposure        = 3 * 100 * (1 << SENSOR_FIX_FRACBITS),
			.cur_fps               = 30 * (1 << SENSOR_FIX_FRACBITS),
			.max_fps               = 30 * (1 << SENSOR_FIX_FRACBITS),
			.min_fps               = 5 * (1 << SENSOR_FIX_FRACBITS),
			.min_afps              = 5 * (1 << SENSOR_FIX_FRACBITS),
			.int_update_delay_frm  = 1,
			.gain_update_delay_frm = 1,
		},
		.mipi_info = {
			.mipi_lane = 2,
		},
		.preg_data      = af0130_init_setting_linear_1M,
		.reg_data_count = ARRAY_SIZE(af0130_init_setting_linear_1M),
	},

};

static int af0130_get_clk(struct af0130 *sensor, void *clk)
{
	struct vvcam_clk_s vvcam_clk;
	int ret = 0;
	vvcam_clk.sensor_mclk = clk_get_rate(sensor->sensor_clk);
	vvcam_clk.csi_max_pixel_clk = sensor->ocp.max_pixel_frequency;
	ret = copy_to_user(clk, &vvcam_clk, sizeof(struct vvcam_clk_s));
	if (ret != 0)
		ret = -EINVAL;
	return ret;
}

static int af0130_power_on(struct af0130 *sensor)
{
	int ret;
	pr_debug("enter %s\n", __func__);

	if (gpio_is_valid(sensor->pwn_gpio))
		gpio_set_value_cansleep(sensor->pwn_gpio, 1);

	ret = clk_prepare_enable(sensor->sensor_clk);
	if (ret < 0)
		pr_err("%s: enable sensor clk fail\n", __func__);

	return ret;
}

static int af0130_power_off(struct af0130 *sensor)
{
	pr_debug("enter %s\n", __func__);
	if (gpio_is_valid(sensor->pwn_gpio))
		gpio_set_value_cansleep(sensor->pwn_gpio, 0);
	clk_disable_unprepare(sensor->sensor_clk);

	return 0;
}

static int af0130_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct af0130 *sensor = client_to_af0130(client);

	pr_debug("enter %s\n", __func__);
	if (on)
		af0130_power_on(sensor);
	else
		af0130_power_off(sensor);

	return 0;
}
//default 16bits data write by i2c
static s32 af0130_write_reg(struct af0130 *sensor, u16 reg, u16 val)
{
	struct device *dev = &sensor->i2c_client->dev;
	u8 au8Buf[4] = { reg >> 8, reg & 0xff, val >> 8, val & 0xff };

	if (i2c_master_send(sensor->i2c_client, au8Buf, 4) < 0) {
		dev_err(dev, "Write reg error: reg=%x, val=%x\n", reg, val);
		return -1;
	}
	return 0;
}

static s32 af0130_write_reg_special(struct af0130 *sensor, u16 reg, u16 val, u16 bytes)
{
	struct device *dev = &sensor->i2c_client->dev;
	u8 au8Buf[4] = { reg >> 8, reg & 0xff, val >> 8, val & 0xff };;

	if(bytes == 1){
		au8Buf[2] = val & 0xff;
	}
	
	if (i2c_master_send(sensor->i2c_client, au8Buf, (2+bytes)) < 0) {
		dev_err(dev, "Write reg special error: reg=%x, val=%x\n", reg, val);
		return -1;
	}
	return 0;
}


static s32 af0130_read_reg(struct af0130 *sensor, u16 reg, u16 *val)
{
	struct device *dev = &sensor->i2c_client->dev;
	u8 au8RegBuf[2] = { reg >> 8, reg & 0xff };
	u8 au8RdVal[2] = {0};

	if (i2c_master_send(sensor->i2c_client, au8RegBuf, 2) != 2) {
		dev_err(dev, "Read reg error: reg=%x\n", reg);
		return -1;
	}

	if (i2c_master_recv(sensor->i2c_client, au8RdVal, 2) != 2) {
		dev_err(dev, "Read reg error: reg=%x, val=%x %x\n",
                        reg, au8RdVal[0], au8RdVal[1]);
		return -1;
	}

	*val = ((u16)au8RdVal[0] << 8) | (u16)au8RdVal[1];

	return 0;
}

static int af0130_read_register_chunk(struct i2c_client* client, __u8* buffer, __u8 buffer_size, __u16 register_address)
{
        struct i2c_msg msgs[2] = {};
        int ret = 0;
	u16 addr = ((register_address &0xff)<<8) + ((register_address >>8) &0xff);
        msgs[0].addr = client->addr;
        msgs[0].flags = 0;
        msgs[0].buf = (__u8 *)&addr;
        msgs[0].len = 2;//sizeof(register_address);

        msgs[1].addr = client->addr;
        msgs[1].flags = I2C_M_RD;
        msgs[1].buf = buffer;
        msgs[1].len = buffer_size;

        ret = i2c_transfer(client->adapter, msgs, 2);
        if (ret < 0) {
                pr_err("i2c_transfer() failed: %d\n", ret);
                return ret;
        }

        if (ret != 2) {
                pr_err("i2c_transfer() incomplete");
                return -EIO;
        }

        return msgs[1].len;
}

static int af0130_stream_on(struct af0130 *sensor)
{
	int ret;
	//u16 val = AF0130_MODE_STREAMING;
	printk("af0130_stream_on\n");	
	ret = af0130_write_reg_special(sensor, AF0130_REG_CTRL_MODE, AF0130_MODE_STREAMING, AF0130_REG_VALUE_08BIT);

	return ret;
}

static int af0130_stream_off(struct af0130 *sensor)
{
	int ret;
	//u16 val = 0;

	ret = af0130_write_reg_special(sensor, AF0130_REG_CTRL_MODE, AF0130_MODE_SW_STANDBY, AF0130_REG_VALUE_08BIT);

	return ret;
}

static int af0130_write_reg_arry(struct af0130 *sensor,
				    struct vvcam_sccb_data_with_length_s *mode_setting,
				    s32 size)
{
	register u16 reg_addr = 0;
	register u16 data = 0;
	register u16 bytes = 0;
	int i, retval = 0;

	for (i = 0; i < size; ++i, ++mode_setting) {
		if (unlikely(mode_setting->addr == REG_DELAY)) {
			usleep_range(mode_setting->data*1000, mode_setting->data * 2000);
		} else {
			reg_addr = mode_setting->addr;
			data = mode_setting->data;
			bytes = mode_setting->len;
			//printk("af0130_write_reg_arry, reg=0x%x, data=0x%x, bytes=%d\n", reg_addr, data, bytes);
			retval = af0130_write_reg_special(sensor, reg_addr, data, bytes);
			if (retval < 0)
				break;
		}
	}

	af0130_stream_off(sensor);

	return retval;
}
static int af0130_write_reg_arry_16bits(struct af0130 *sensor,
                                    u16 *mode_setting,
                                    s32 size)
{
        register u16 reg_addr = 0;
        register u16 data = 0;
        register u16 bytes = 0;
        int i, retval = 0;

        for (i = 0; i < size; i++) {
                        reg_addr = __cpu_to_be16(mode_setting[i*2]);//((mode_setting[i*2]>>8)&0xff) + ((mode_setting[i*2]&0xff) <<8);
                        data = __cpu_to_be16(mode_setting[i*2+1]);//((mode_setting[i*2+1]>>8)&0xff) + ((mode_setting[i*2+1]&0xff) <<8);
                        bytes = 2;
                        printk("af0130_write_reg_arry_16bits, %dth reg=0x%x, data=0x%x, bytes=%d\n", i, reg_addr, data, bytes);
                        retval = af0130_write_reg_special(sensor, reg_addr, data, bytes);
                        if (retval < 0)
                                break;
        }

        return retval;
}
static int af0130_query_capability(struct af0130 *sensor, void *arg)
{
	struct v4l2_capability *pcap = (struct v4l2_capability *)arg;

	strcpy((char *)pcap->driver, "af0130");
	sprintf((char *)pcap->bus_info, "csi%d",sensor->csi_id);
	if(sensor->i2c_client->adapter) {
		pcap->bus_info[VVCAM_CAP_BUS_INFO_I2C_ADAPTER_NR_POS] =
			(__u8)sensor->i2c_client->adapter->nr;
	} else {
		pcap->bus_info[VVCAM_CAP_BUS_INFO_I2C_ADAPTER_NR_POS] = 0xFF;
	}
	return 0;
}

static int af0130_query_supports(struct af0130 *sensor, void* parry)
{
	int ret = 0;
	struct vvcam_mode_info_array_s *psensor_mode_arry = parry;
	uint32_t support_counts = ARRAY_SIZE(paf0130_mode_info);

	ret = copy_to_user(&psensor_mode_arry->count, &support_counts, sizeof(support_counts));
	ret |= copy_to_user(&psensor_mode_arry->modes, paf0130_mode_info,
			   sizeof(paf0130_mode_info));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;
}

static int af0130_get_sensor_id(struct af0130 *sensor, void* pchip_id)
{
	int ret = 0;
	u16 chip_id;

	ret = af0130_read_reg(sensor, 0x3000, &chip_id);
	ret = copy_to_user(pchip_id, &chip_id, sizeof(u16));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;
}

static int af0130_get_reserve_id(struct af0130 *sensor, void* preserve_id)
{
	int ret = 0;
	u16 reserve_id = 0x2770;
	ret = copy_to_user(preserve_id, &reserve_id, sizeof(u16));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;
}

static int af0130_get_sensor_mode(struct af0130 *sensor, void* pmode)
{
	int ret = 0;
	ret = copy_to_user(pmode, &sensor->cur_mode,
		sizeof(struct vvcam_mode_info_s));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;
}

static int af0130_set_sensor_mode(struct af0130 *sensor, void* pmode)
{
	int ret = 0;
	int i = 0;
	struct vvcam_mode_info_s sensor_mode;

	ret = copy_from_user(&sensor_mode, pmode,
		sizeof(struct vvcam_mode_info_s));
	if (ret != 0)
		return -ENOMEM;
	for (i = 0; i < ARRAY_SIZE(paf0130_mode_info); i++) 
    {
		if (paf0130_mode_info[i].index == sensor_mode.index) 
        {
			memcpy(&sensor->cur_mode, &paf0130_mode_info[i],sizeof(struct vvcam_mode_info_s));
			return 0;
		}
	}
	return -ENXIO;
}

static int af0130_set_exp(struct af0130 *sensor, u32 exp)
{
	int ret = 0;
	printk("set exp: %d\n", exp);
	ret |= af0130_write_reg(sensor, 0x0202, exp);

	return ret;
}

static int af0130_set_gain(struct af0130 *sensor, u32 gain)
{
	int ret = 0;
	u16 new_gain = 0;
	unsigned int i = 0;

	pr_info("%s : %d\n", __func__, gain);

	for(i = 0; i < GAIN_TABLE_SIZE; i++){
		if(gain <= af0130_gain_table[i].times){
			new_gain = af0130_gain_table[i].code;
			break;
		}
	}
	if (i == GAIN_TABLE_SIZE){
		printk("wrong times: %d\n", new_gain);
		new_gain = AF0130_GAIN_MAX;
	}
	if (new_gain > AF0130_GAIN_MAX) {
		new_gain = AF0130_GAIN_MAX;
	}else if (new_gain < AF0130_GAIN_MIN) {
                new_gain = AF0130_GAIN_MIN;
        }
	printk("set gain code final: %d\n", new_gain);
	ret = af0130_write_reg(sensor, AF0130_REG_GAIN, new_gain);
    return ret;
}

static int af0130_set_fps(struct af0130 *sensor, u32 fps)
{
	u32 vts;
	int ret = 0;

	if (fps > sensor->cur_mode.ae_info.max_fps) {
		fps = sensor->cur_mode.ae_info.max_fps;
	}
	else if (fps < sensor->cur_mode.ae_info.min_fps) {
		fps = sensor->cur_mode.ae_info.min_fps;
	}
	vts = sensor->cur_mode.ae_info.max_fps *
	      sensor->cur_mode.ae_info.def_frm_len_lines / fps;

	ret |= af0130_write_reg(sensor, 0x0340, vts);
	sensor->cur_mode.ae_info.cur_fps = fps;

	if (sensor->cur_mode.hdr_mode == SENSOR_MODE_LINEAR) {
		sensor->cur_mode.ae_info.max_integration_line = vts - 1;
	} else {
		if (sensor->cur_mode.stitching_mode ==
		    SENSOR_STITCHING_DUAL_DCG){
			sensor->cur_mode.ae_info.max_vsintegration_line = 44;
			sensor->cur_mode.ae_info.max_integration_line = vts -
				4 - sensor->cur_mode.ae_info.max_vsintegration_line;
		} else {
			sensor->cur_mode.ae_info.max_integration_line = vts - 1;
		}
	}
	sensor->cur_mode.ae_info.curr_frm_len_lines = vts;
	return ret;
}

static int af0130_get_fps(struct af0130 *sensor, u32 *pfps)
{
	*pfps = sensor->cur_mode.ae_info.cur_fps;
	return 0;

}

static int af0130_set_test_pattern(struct af0130 *sensor, void * arg)
{

	int ret = 0;
#if 0
	struct sensor_test_pattern_s test_pattern;

	ret = copy_from_user(&test_pattern, arg, sizeof(test_pattern));
	if (ret != 0)
		return -ENOMEM;
	if (test_pattern.enable) {
		switch (test_pattern.pattern) {
		case 0:
			ret |= af0130_write_reg(sensor, 0x0600, 0x0001);
			break;
		case 1:
			ret |= af0130_write_reg(sensor, 0x0600, 0x0002);
			break;
		case 2:
			ret |= af0130_write_reg(sensor, 0x0600, 0x0003);
			break;
		default:
			ret = -1;
			break;
		}
	}
#endif
	return ret;
}

static int af0130_get_lens(struct af0130 *sensor, void * arg) {

	vvcam_lens_t *pfocus_lens = (vvcam_lens_t *)arg;

	if (!arg)
		return -ENOMEM;

	if (strlen(sensor->focus_lens.name) == 0)
		return -1;

	return copy_to_user(pfocus_lens, &sensor->focus_lens, sizeof(vvcam_lens_t));
}

static int af0130_get_format_code(struct af0130 *sensor, u32 *code)
{
	switch (sensor->cur_mode.bayer_pattern) {
	case BAYER_RGGB:
		if (sensor->cur_mode.bit_width == 8) {
			*code = MEDIA_BUS_FMT_SRGGB8_1X8;
		} else if (sensor->cur_mode.bit_width == 10) {
			*code = MEDIA_BUS_FMT_SRGGB10_1X10;
		} else {
			*code = MEDIA_BUS_FMT_SRGGB12_1X12;
		}
		break;
	case BAYER_GRBG:
		if (sensor->cur_mode.bit_width == 8) {
			*code = MEDIA_BUS_FMT_SGRBG8_1X8;
		} else if (sensor->cur_mode.bit_width == 10) {
			*code = MEDIA_BUS_FMT_SGRBG10_1X10;
		} else {
			*code = MEDIA_BUS_FMT_SGRBG12_1X12;
		}
		break;
	case BAYER_GBRG:
		if (sensor->cur_mode.bit_width == 8) {
			*code = MEDIA_BUS_FMT_SGBRG8_1X8;
		} else if (sensor->cur_mode.bit_width == 10) {
			*code = MEDIA_BUS_FMT_SGBRG10_1X10;
		} else {
			*code = MEDIA_BUS_FMT_SGBRG12_1X12;
		}
		break;
	case BAYER_BGGR:
		if (sensor->cur_mode.bit_width == 8) {
			*code = MEDIA_BUS_FMT_SBGGR8_1X8;
		} else if (sensor->cur_mode.bit_width == 10) {
			*code = MEDIA_BUS_FMT_SBGGR10_1X10;
		} else {
			*code = MEDIA_BUS_FMT_SBGGR12_1X12;
		}
		break;
	case BAYER_Y12:
                if (sensor->cur_mode.bit_width == 12) {
                        *code = MEDIA_BUS_FMT_Y12_1X12;
                }
                break;
	default:
		/*nothing need to do*/
		break;
	}
	return 0;
}


static int af0130_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct af0130 *sensor = client_to_af0130(client);
    int ret;
	
	pr_debug("enter %s\n", __func__);
	printk("af0130_s_stream: %d\n", enable);
	if (enable) {
        ret = af0130_stream_on(sensor);
        if (ret < 0)
            return ret;
    } else {
        ret = af0130_stream_off(sensor);
        if (ret < 0)
            return ret;
    }

	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
static int af0130_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_state *state,
				 struct v4l2_subdev_mbus_code_enum *code)
#else
static int af0130_enum_mbus_code(struct v4l2_subdev *sd,
			         struct v4l2_subdev_pad_config *cfg,
			         struct v4l2_subdev_mbus_code_enum *code)
#endif
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct af0130 *sensor = client_to_af0130(client);

	u32 cur_code = MEDIA_BUS_FMT_SBGGR12_1X12;

	if (code->index > 0)
		return -EINVAL;
	af0130_get_format_code(sensor,&cur_code);
	code->code = cur_code;

	return 0;
}

static int read_eeprom_pga_data(struct af0130 *sensor);
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
static int af0130_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_state *state,
			  struct v4l2_subdev_format *fmt)
#else
static int af0130_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)

#endif
{
	int ret = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct af0130 *sensor = client_to_af0130(client);
	int i = 0;

	mutex_lock(&sensor->lock);
	pr_info("enter %s\n", __func__);
	if ((fmt->format.width != sensor->cur_mode.size.bounds_width) ||
	    (fmt->format.height != sensor->cur_mode.size.bounds_height)) {
		pr_err("%s:set sensor format %dx%d met different size, choose mode now\n",
			__func__,fmt->format.width,fmt->format.height);
		//mutex_unlock(&sensor->lock);
		//return -EINVAL;
		for (i = 0; i < ARRAY_SIZE(paf0130_mode_info); i++)
		{
			if ((paf0130_mode_info[i].size.bounds_width == fmt->format.width)
					&& (paf0130_mode_info[i].size.bounds_height == fmt->format.height))
			{
				memcpy(&sensor->cur_mode, &paf0130_mode_info[i],sizeof(struct vvcam_mode_info_s));
				if(i == 0){
					used_freq_num = 1;
				}else if(i == 1){
					used_freq_num = 0;
				}
				break;
			}
		}
		if(i == ARRAY_SIZE(paf0130_mode_info)){
			pr_err("%s:set sensor format %dx%d didn't find matching modes\n",
                        __func__,fmt->format.width,fmt->format.height);
			return -EINVAL;
		}
	}
	read_eeprom_pga_data(sensor);
	ret |= af0130_write_reg_arry(sensor,
		sensor->cur_mode.preg_data,
		sensor->cur_mode.reg_data_count);
	
	if (ret < 0) {
		pr_err("%s:af0130_write_reg_arry error\n",__func__);
		mutex_unlock(&sensor->lock);
		return -EINVAL;
	}
	if(sensor->has_eeprom && sensor->eeprom_data_ready){
		ret |= af0130_write_reg_arry_16bits(sensor,
		        (u16*)(eeprom_buf),
		        eeprom_mode_settings[used_freq_num].total_size/4);

		if (ret < 0) {
		        pr_err("%s:af0130_write_reg_arry of calibration data error\n",__func__);
		}
        }
	af0130_get_format_code(sensor, &fmt->format.code);
	fmt->format.field = V4L2_FIELD_NONE;
	sensor->format = fmt->format;
	mutex_unlock(&sensor->lock);
	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
static int af0130_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_state *state,
			  struct v4l2_subdev_format *fmt)
#else
static int af0130_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)
#endif
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct af0130 *sensor = client_to_af0130(client);

	mutex_lock(&sensor->lock);
	fmt->format = sensor->format;
	mutex_unlock(&sensor->lock);
	return 0;
}

static long af0130_priv_ioctl(struct v4l2_subdev *sd,
                              unsigned int cmd,
                              void *arg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct af0130 *sensor = client_to_af0130(client);
	long ret = 0;
	struct vvcam_sccb_data_s sensor_reg;
	uint32_t value = 0;

	mutex_lock(&sensor->lock);
	switch (cmd){
	case VVSENSORIOC_S_POWER:
		ret = 0;
		break;
	case VVSENSORIOC_S_CLK:
		ret = 0;
		break;
	case VVSENSORIOC_G_CLK:
		ret = af0130_get_clk(sensor,arg);
		break;
	case VVSENSORIOC_RESET:
		ret = 0;
		break;
	case VIDIOC_QUERYCAP:
		ret = af0130_query_capability(sensor, arg);
		break;
	case VVSENSORIOC_QUERY:
		ret = af0130_query_supports(sensor, arg);
		break;
	case VVSENSORIOC_G_CHIP_ID:
		ret = af0130_get_sensor_id(sensor, arg);
		break;
	case VVSENSORIOC_G_RESERVE_ID:
		ret = af0130_get_reserve_id(sensor, arg);
		break;
	case VVSENSORIOC_G_SENSOR_MODE:
		ret = af0130_get_sensor_mode(sensor, arg);
		break;
	case VVSENSORIOC_S_SENSOR_MODE:
		ret = af0130_set_sensor_mode(sensor, arg);
		break;
	case VVSENSORIOC_S_STREAM:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= af0130_s_stream(&sensor->subdev, value);
		break;
	case VVSENSORIOC_WRITE_REG:
		ret = copy_from_user(&sensor_reg, arg,
			sizeof(struct vvcam_sccb_data_s));
		ret |= af0130_write_reg(sensor, sensor_reg.addr,
			sensor_reg.data);
		break;
	case VVSENSORIOC_READ_REG:
		ret = copy_from_user(&sensor_reg, arg, sizeof(struct vvcam_sccb_data_s));
		ret |= af0130_read_reg(sensor, (u16)sensor_reg.addr, (u16 *)&sensor_reg.data);
		ret |= copy_to_user(arg, &sensor_reg, sizeof(struct vvcam_sccb_data_s));
		break;
	case VVSENSORIOC_S_EXP:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= af0130_set_exp(sensor, value);
		break;
	case VVSENSORIOC_S_GAIN:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= af0130_set_gain(sensor, value);
		break;
	case VVSENSORIOC_S_FPS:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= af0130_set_fps(sensor, value);
		break;
	case VVSENSORIOC_G_FPS:
		ret = af0130_get_fps(sensor, &value);
		ret |= copy_to_user(arg, &value, sizeof(value));
		break;
	case VVSENSORIOC_S_TEST_PATTERN:
		ret= af0130_set_test_pattern(sensor, arg);
		break;
	case VVSENSORIOC_G_LENS:
		ret = af0130_get_lens(sensor, arg);
		break;
	default:
		break;
	}

	mutex_unlock(&sensor->lock);
	return ret;
}

static struct v4l2_subdev_video_ops af0130_subdev_video_ops = {
	.s_stream = af0130_s_stream,
};

static const struct v4l2_subdev_pad_ops af0130_subdev_pad_ops = {
	.enum_mbus_code = af0130_enum_mbus_code,
	.set_fmt = af0130_set_fmt,
	.get_fmt = af0130_get_fmt,
};

static struct v4l2_subdev_core_ops af0130_subdev_core_ops = {
	.s_power = af0130_s_power,
	.ioctl = af0130_priv_ioctl,
};

static struct v4l2_subdev_ops af0130_subdev_ops = {
	.core  = &af0130_subdev_core_ops,
	.video = &af0130_subdev_video_ops,
	.pad   = &af0130_subdev_pad_ops,
};

static int af0130_link_setup(struct media_entity *entity,
			     const struct media_pad *local,
			     const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations af0130_sd_media_ops = {
	.link_setup = af0130_link_setup,
};

static int af0130_regulator_enable(struct af0130 *sensor)
{
	int ret = 0;
	struct device *dev = &(sensor->i2c_client->dev);

	pr_debug("enter %s\n", __func__);

	if (sensor->io_regulator) {
		regulator_set_voltage(sensor->io_regulator,
				      AF0130_VOLTAGE_DIGITAL_IO,
				      AF0130_VOLTAGE_DIGITAL_IO);
		ret = regulator_enable(sensor->io_regulator);
		if (ret < 0) {
			dev_err(dev, "set io voltage failed\n");
			return ret;
		}
	}

	if (sensor->analog_regulator) {
		regulator_set_voltage(sensor->analog_regulator,
				      AF0130_VOLTAGE_ANALOG,
				      AF0130_VOLTAGE_ANALOG);
		ret = regulator_enable(sensor->analog_regulator);
		if (ret) {
			dev_err(dev, "set analog voltage failed\n");
			goto err_disable_io;
		}

	}

	if (sensor->core_regulator) {
		regulator_set_voltage(sensor->core_regulator,
				      AF0130_VOLTAGE_DIGITAL_CORE,
				      AF0130_VOLTAGE_DIGITAL_CORE);
		ret = regulator_enable(sensor->core_regulator);
		if (ret) {
			dev_err(dev, "set core voltage failed\n");
			goto err_disable_analog;
		}
	}

	return 0;

err_disable_analog:
	regulator_disable(sensor->analog_regulator);
err_disable_io:
	regulator_disable(sensor->io_regulator);
	return ret;
}

static void af0130_regulator_disable(struct af0130 *sensor)
{
	int ret = 0;
	struct device *dev = &(sensor->i2c_client->dev);

	if (sensor->core_regulator) {
		ret = regulator_disable(sensor->core_regulator);
		if (ret < 0)
			dev_err(dev, "core regulator disable failed\n");
	}

	if (sensor->analog_regulator) {
		ret = regulator_disable(sensor->analog_regulator);
		if (ret < 0)
			dev_err(dev, "analog regulator disable failed\n");
	}

	if (sensor->io_regulator) {
		ret = regulator_disable(sensor->io_regulator);
		if (ret < 0)
			dev_err(dev, "io regulator disable failed\n");
	}
	return ;
}

static int af0130_set_clk_rate(struct af0130 *sensor)
{
	int ret;
	unsigned int clk;

	clk = sensor->mclk;
	clk = min_t(u32, clk, (u32)AF0130_XCLK_MAX);
	clk = max_t(u32, clk, (u32)AF0130_XCLK_MIN);
	sensor->mclk = clk;

	pr_debug("   Setting mclk to %d MHz\n",sensor->mclk / 1000000);
	ret = clk_set_rate(sensor->sensor_clk, sensor->mclk);
	if (ret < 0)
		pr_debug("set rate filed, rate=%d\n", sensor->mclk);
	return ret;
}

static void af0130_reset(struct af0130 *sensor)
{
	pr_debug("enter %s\n", __func__);
	if (!gpio_is_valid(sensor->rst_gpio))
		return;

	gpio_set_value_cansleep(sensor->rst_gpio, 0);
	msleep(20);

	gpio_set_value_cansleep(sensor->rst_gpio, 1);
	msleep(20);

	return;
}

static int af0130_eeprom_device_release(struct af0130 *priv)
{

	if (priv->eeprom.i2c_client != NULL) {
		i2c_unregister_device(priv->eeprom.i2c_client);
		priv->eeprom.i2c_client = NULL;
	}

	return 0;
}

static int read_eeprom_pga_data(struct af0130 *sensor){
	u8 pga_head[PGA_HEAD_SIZE] = {0};
	u8* tmp_buf = eeprom_buf;
	mode_list* mode_setting = &eeprom_mode_settings[used_freq_num];
	int ret = 0;
	u16 i = 0;
	u16 j = 0;
	u8 read_bytes_once = 0;
	u16 length = 0;
	u16 naddr = PGA_START_ADDR;
	u16 left_size = mode_setting->total_size;
	u16 max_eeprom_size = TL_EEPROM_CMN_AREA_SIZE_;//0x1B82;//end of eeprom address.

	memset(eeprom_buf, 0, MAX_EEPROM_BUF_SIZE);

	for(i = 0; i < mode_setting->list_len; i++){
		while(naddr < max_eeprom_size){
			memset(pga_head, 0, PGA_HEAD_SIZE);
			printk("start reading head.\n");
			ret = af0130_read_register_chunk(sensor->eeprom.i2c_client, pga_head, PGA_HEAD_SIZE, naddr);
			printk("done reading head.\n");
			if(ret != PGA_HEAD_SIZE){
		        	printk("ERR read pga_head, wrong size got: %d\n", ret);
					return -EINVAL;
			}else{
				if(pga_head[0] == PGA_TYPE){//PGA head found.
		                	length = pga_head[1];
		                	length = (length<<8) + pga_head[2];
		                	printk("GOT PGA length: %d, label str: %s\n", length, pga_head+3);
					if(strncmp((pga_head+3), mode_setting->sub_name[i], strlen(mode_setting->sub_name[i])) == 0){
						printk("--USED_FREQ_STRING-- len: %d matches, start reading data\n", (int)strlen(pga_labels[used_freq_num]));
						sensor->data_len = length - 12;//the head has length 15 bytes. The 1type and 2length bytes are not counted in var length.
						j = 0;
						naddr += 15;//the head length.
						while(j < (sensor->data_len)){
		        				if((sensor->data_len-j) >= 34)
		                				read_bytes_once = 34;
		        				else
		                				read_bytes_once = sensor->data_len-j;
		        				printk("read bytes once: %d\n", read_bytes_once);
		        				ret = af0130_read_register_chunk(sensor->eeprom.i2c_client, tmp_buf+j, read_bytes_once, naddr+j);
		        				if(ret != read_bytes_once){
		                				printk("ERR read calib data, wrong size got: %d at j: %d\n", ret, j);
								af0130_eeprom_device_release(sensor);
		                				return -EINVAL;
		        				}
		        				j += read_bytes_once;
						}
						left_size -= (length-12);//drop the 12 bytes head.
						if(left_size < 0){
							printk("size mismatch, cur GOT PGA length: %d, left size: %d\n", (length-12), left_size);
							return -EINVAL;
						}
						tmp_buf += (length-12);
						naddr = PGA_START_ADDR;//0x05;
						break;//one sub_name's data got, start new search for next sub_name from beginning of eeprom;
					} else{
						printk("name mismatch, find next\n");
						naddr += (length+3);//move to next data section for searching current sub_name.
					}
		        	}
		        	else
		        	{
		        		pr_err("read_eeprom_pga_data: wrong PGA_HEAD value 0x%04x != 0x%04x \n", pga_head[0], PGA_TYPE);
		        	}		        	
			}
			//the type and length bytes are not counted in length.
		}
	}
	if(left_size != 0){
		printk("size mismatch, cur left size: %d\n", left_size);
		return -EINVAL;
	}
	sensor->eeprom_data_ready = true;
#if 0
	for(i = 0; i< sensor->data_len; i++){
		printk("got %dth buf: %x\n", i, eeprom_buf[i]);
	}
#endif
	return 0;
}

static int af0130_eeprom_device_init(struct af0130 *sensor){
	char *dev_name = "eeprom_af0130";
	u8 head[5] = {0};
	int ret = 0;

	sensor->data_len = 0;
	sensor->eeprom_data_ready = false;
	if(!sensor->has_eeprom){
		printk("don't has eeprom in dts~\n");
		return -EINVAL;
	}
	//printk("already has eeprom in dts now.\n");
	sensor->eeprom.adap = i2c_get_adapter(sensor->i2c_client->adapter->nr);
	memset(&sensor->eeprom.brd, 0, sizeof(sensor->eeprom.brd));
	strncpy(sensor->eeprom.brd.type, dev_name,
                                sizeof(sensor->eeprom.brd.type));
	sensor->eeprom.brd.addr = AF0130_EEPROM_ADDRESS;
	sensor->eeprom.i2c_client = i2c_new_client_device(
                                sensor->eeprom.adap, &sensor->eeprom.brd);

	u16 head_addr = 0x0;
	ret = af0130_read_register_chunk(sensor->eeprom.i2c_client, head, 5, head_addr);
	if( ret > 0){
		printk("head read: %x %x %x %x %x\n", head[0], head[1],head[2],head[3],head[4]);
		if(!((head[0]==0x4d) && (head[1]==0x49) && (head[2]==0x45)&& (head[3]==0x45)&& (head[4]==0x01))){
			af0130_eeprom_device_release(sensor);
			printk("eeprom head is wrong~\n");
			return -EINVAL;
		}	
	}else{
		printk("read head failure\n");
		af0130_eeprom_device_release(sensor);
		return ret; 
	}
	
	
	return read_eeprom_pga_data(sensor);
}

static int af0130_retrieve_capture_properties(
			struct af0130 *sensor,
			struct af0130_capture_properties* ocp)
{
	struct device *dev = &sensor->i2c_client->dev;
	__u64 mlf = 0;
	__u64 mpf = 0;
	__u64 mdr = 0;

	struct device_node *ep;
	int ret;
	/*Collecting the information about limits of capture path
	* has been centralized to the sensor
	* * also into the sensor endpoint itself.
	*/

	ep = of_graph_get_next_endpoint(dev->of_node, NULL);
	if (!ep) {
		dev_err(dev, "missing endpoint node\n");
		return -ENODEV;
	}

	/*ret = fwnode_property_read_u64(of_fwnode_handle(ep),
		"max-lane-frequency", &mlf);
	if (ret || mlf == 0) {
		dev_dbg(dev, "no limit for max-lane-frequency\n");
	}*/
	ret = fwnode_property_read_u64(of_fwnode_handle(ep),
	        "max-pixel-frequency", &mpf);
	if (ret || mpf == 0) {
	        dev_dbg(dev, "no limit for max-pixel-frequency\n");
	}

	/*ret = fwnode_property_read_u64(of_fwnode_handle(ep),
	        "max-data-rate", &mdr);
	if (ret || mdr == 0) {
	        dev_dbg(dev, "no limit for max-data_rate\n");
	}*/

	ocp->max_lane_frequency = mlf;
	ocp->max_pixel_frequency = mpf;
	ocp->max_data_rate = mdr;

	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
static int af0130_probe(struct i2c_client *client)
#else
static int af0130_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
#endif
{
	int retval;
	struct device *dev = &client->dev;
	struct v4l2_subdev *sd;
	struct af0130 *sensor;
    u16 chip_id;
	struct device_node *lens_node;

	pr_info("enter %s\n", __func__);

	sensor = devm_kmalloc(dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;
	memset(sensor, 0, sizeof(*sensor));

	sensor->i2c_client = client;

	sensor->pwn_gpio = of_get_named_gpio(dev->of_node, "pwn-gpios", 0);
	if (!gpio_is_valid(sensor->pwn_gpio))
		dev_warn(dev, "No sensor pwdn pin available");
	else {
		retval = devm_gpio_request_one(dev, sensor->pwn_gpio,
						GPIOF_OUT_INIT_HIGH,
						"af0130_mipi_pwdn");
		if (retval < 0) {
			dev_warn(dev, "Failed to set power pin\n");
			dev_warn(dev, "retval=%d\n", retval);
			return retval;
		}
	}

	sensor->rst_gpio = of_get_named_gpio(dev->of_node, "rst-gpios", 0);
	if (!gpio_is_valid(sensor->rst_gpio))
		dev_warn(dev, "No sensor reset pin available");
	else {
		retval = devm_gpio_request_one(dev, sensor->rst_gpio,
						GPIOF_OUT_INIT_HIGH,
						"af0130_mipi_reset");
		if (retval < 0) {
			dev_warn(dev, "Failed to set reset pin\n");
			return retval;
		}
	}

	sensor->sensor_clk = devm_clk_get(dev, "csi_mclk");
	if (IS_ERR(sensor->sensor_clk)) {
		sensor->sensor_clk = NULL;
		dev_err(dev, "clock-frequency missing or invalid\n");
		return PTR_ERR(sensor->sensor_clk);
	}

	retval = of_property_read_u32(dev->of_node, "mclk", &(sensor->mclk));
	if (retval) {
		dev_err(dev, "mclk missing or invalid\n");
		return retval;
	}

	retval = of_property_read_u32(dev->of_node, "mclk_source",
				(u32 *)&(sensor->mclk_source));
	if (retval) {
		dev_err(dev, "mclk_source missing or invalid\n");
		return retval;
	}

	retval = of_property_read_u32(dev->of_node, "csi_id", &(sensor->csi_id));
	if (retval) {
		dev_err(dev, "csi id missing or invalid\n");
		return retval;
	}

	lens_node = of_parse_phandle(dev->of_node, "lens-focus", 0);
	if (lens_node) {
		retval = of_property_read_u32(lens_node, "id", &sensor->focus_lens.id);
		if (retval) {
			dev_err(dev, "lens-focus id missing or invalid\n");
			return retval;
		}
		memcpy(sensor->focus_lens.name, lens_node->name, strlen(lens_node->name));
	}

	retval = af0130_retrieve_capture_properties(sensor,&sensor->ocp);
	if (retval) {
		dev_warn(dev, "retrive capture properties error\n");
	}
	sensor->has_eeprom = of_property_read_bool(dev->of_node, "has_eeprom");

	sensor->io_regulator = devm_regulator_get(dev, "DOVDD");
	if (IS_ERR(sensor->io_regulator)) {
		dev_err(dev, "cannot get io regulator\n");
		return PTR_ERR(sensor->io_regulator);
	}

	sensor->core_regulator = devm_regulator_get(dev, "DVDD");
	if (IS_ERR(sensor->core_regulator)) {
		dev_err(dev, "cannot get core regulator\n");
		return PTR_ERR(sensor->core_regulator);
	}

	sensor->analog_regulator = devm_regulator_get(dev, "AVDD");
	if (IS_ERR(sensor->analog_regulator)) {
		dev_err(dev, "cannot get analog  regulator\n");
		return PTR_ERR(sensor->analog_regulator);
	}

	retval = af0130_regulator_enable(sensor);
	if (retval) {
		dev_err(dev, "regulator enable failed\n");
		return retval;
	}

	af0130_set_clk_rate(sensor);
	retval = clk_prepare_enable(sensor->sensor_clk);
	if (retval < 0) {
		dev_err(dev, "%s: enable sensor clk fail\n", __func__);
		goto probe_err_regulator_disable;
	}
	mdelay(2);

	retval = af0130_power_on(sensor);
	if (retval < 0) {
		dev_err(dev, "%s: sensor power on fail\n", __func__);
		goto probe_err_regulator_disable;
	}

	af0130_reset(sensor);

    af0130_read_reg(sensor, AF0130_CHIP_VERSION_REG, &chip_id);
	if (chip_id != AF0130_CHIP_ID) {
		dev_err(dev, "Sensor AF0130 is not found\n");
        goto probe_err_power_off;
    }
	retval = af0130_eeprom_device_init(sensor);
	if (retval < 0) {
		dev_err(dev, "%s: cannot get has_eeprom\n", __func__);
	}
	sd = &sensor->subdev;
	v4l2_i2c_subdev_init(sd, client, &af0130_subdev_ops);
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	sd->dev = &client->dev;
	sd->entity.ops = &af0130_sd_media_ops;
	sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
	sensor->pads[AF0130_SENS_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;
	retval = media_entity_pads_init(&sd->entity,
				AF0130_SENS_PADS_NUM,
				sensor->pads);
	if (retval < 0)
		goto probe_err_power_off;

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
	retval = v4l2_async_register_subdev_sensor(sd);
#else
	retval = v4l2_async_register_subdev_sensor_common(sd);
#endif
	if (retval < 0) {
		dev_err(&client->dev,"%s--Async register failed, ret=%d\n",
			__func__,retval);
		goto probe_err_free_entiny;
	}

	memcpy(&sensor->cur_mode, &paf0130_mode_info[0],
			sizeof(struct vvcam_mode_info_s));

	mutex_init(&sensor->lock);

	pr_info("%s camera mipi af0130, is found\n", __func__);

	return 0;

probe_err_free_entiny:
	media_entity_cleanup(&sd->entity);

probe_err_power_off:
	af0130_power_off(sensor);

probe_err_regulator_disable:
	af0130_regulator_disable(sensor);

	return retval;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static int af0130_remove(struct i2c_client *client)
#else
static void af0130_remove(struct i2c_client *client)
#endif
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct af0130 *sensor = client_to_af0130(client);

	pr_info("enter %s\n", __func__);

	v4l2_async_unregister_subdev(sd);
	media_entity_cleanup(&sd->entity);
	af0130_power_off(sensor);
	af0130_regulator_disable(sensor);
	af0130_eeprom_device_release(sensor);
	mutex_destroy(&sensor->lock);

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
	return 0;
#else
#endif
}

static int __maybe_unused af0130_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct af0130 *sensor = client_to_af0130(client);

	sensor->resume_status = sensor->stream_status;
	if (sensor->resume_status) {
		af0130_s_stream(&sensor->subdev,0);
	}

	return 0;
}

static int __maybe_unused af0130_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct af0130 *sensor = client_to_af0130(client);

	if (sensor->resume_status) {
		af0130_s_stream(&sensor->subdev,1);
	}

	return 0;
}

static const struct dev_pm_ops af0130_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(af0130_suspend, af0130_resume)
};

static const struct i2c_device_id af0130_id[] = {
	{"af0130", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, af0130_id);

static const struct of_device_id af0130_of_match[] = {
	{ .compatible = "onsemi,af0130" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, af0130_of_match);

static struct i2c_driver af0130_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name  = "af0130",
		.pm = &af0130_pm_ops,
		.of_match_table	= af0130_of_match,
	},
	.probe  = af0130_probe,
	.remove = af0130_remove,
	.id_table = af0130_id,
};


module_i2c_driver(af0130_i2c_driver);
MODULE_DESCRIPTION("AF0130 MIPI Camera Subdev Driver");
MODULE_LICENSE("GPL");
