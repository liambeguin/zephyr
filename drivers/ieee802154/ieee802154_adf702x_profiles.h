#if 1
enum adf7024_profile {
	PROFILE_A = 0,
	PROFILE_B = 1,
	PROFILE_C = 2,
	PROFILE_D = 3,
	PROFILE_E = 4,
	PROFILE_F = 5,
};

static int adf7024_regs_set_profile(const struct device *dev, enum adf7024_profile profile)
{
	struct adf702x_context *ctx = dev->data;
	// default with AFC ON and FSK mod
	const uint8_t adf7024_radio_profile_regs[][23] = {
		[PROFILE_A] = {
		       //0   1     2     3     4     5     6     7
		       0x60, 0x00, 0x60, 0x20, 0x04, 0x00, 0x00, 0x00,
		       0x37, 0x2B, 0x2F, 0x12, 0x07, 0x00, 0x00, 0x00,
		       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7 },
		[PROFILE_B] = {
		       0x80, 0x01, 0xC8, 0x20, 0x0E, 0x00, 0x00, 0x00,
		       0x37, 0x2B, 0x2F, 0x12, 0x07, 0x00, 0x00, 0x00,
		       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7 },
		[PROFILE_C] = {
		       0xF4, 0x01, 0xFA, 0x20, 0x13, 0x00, 0x00, 0x00,
		       0x37, 0x2B, 0x2F, 0x12, 0x07, 0x00, 0x00, 0x00,
		       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7 },
		[PROFILE_D] = {
		       0xE8, 0x03, 0xFA, 0x20, 0x26, 0x00, 0x00, 0x00,
		       0x37, 0x2B, 0x2F, 0x12, 0x07, 0x00, 0x00, 0x00,
		       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7 },
		[PROFILE_E] = {
		       0xD0, 0x17, 0xF4, 0x20, 0x4B, 0x00, 0x00, 0x80,
		       0x37, 0x2B, 0x2F, 0x12, 0x07, 0x00, 0x00, 0x00,
		       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7 },
		[PROFILE_F] = {
			0xB8, 0x2B, 0xEE, 0x16, 0x70, 0x00, 0x00, 0xC0,
			0x37, 0x2B, 0x2F, 0x12, 0x07, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7 },
	};

	ctx->conf_regs.radio_profile_0  = adf7024_radio_profile_regs[profile][0];
	ctx->conf_regs.radio_profile_1  = adf7024_radio_profile_regs[profile][1];
	ctx->conf_regs.radio_profile_2  = adf7024_radio_profile_regs[profile][2];
	ctx->conf_regs.radio_profile_3  = adf7024_radio_profile_regs[profile][3];
	ctx->conf_regs.radio_profile_4  = adf7024_radio_profile_regs[profile][4];
	ctx->conf_regs.radio_profile_5  = adf7024_radio_profile_regs[profile][5];
	ctx->conf_regs.radio_profile_6  = adf7024_radio_profile_regs[profile][6];
	ctx->conf_regs.radio_profile_7  = adf7024_radio_profile_regs[profile][7];
	ctx->conf_regs.radio_profile_8  = adf7024_radio_profile_regs[profile][8];
	ctx->conf_regs.radio_profile_9  = adf7024_radio_profile_regs[profile][9];
	ctx->conf_regs.radio_profile_10 = adf7024_radio_profile_regs[profile][10];
	ctx->conf_regs.radio_profile_11 = adf7024_radio_profile_regs[profile][11];
	ctx->conf_regs.radio_profile_12 = adf7024_radio_profile_regs[profile][12];
	ctx->conf_regs.radio_profile_13 = adf7024_radio_profile_regs[profile][13];
	ctx->conf_regs.radio_profile_14 = adf7024_radio_profile_regs[profile][14];
	ctx->conf_regs.radio_profile_15 = adf7024_radio_profile_regs[profile][15];
	ctx->conf_regs.radio_profile_16 = adf7024_radio_profile_regs[profile][16];
	ctx->conf_regs.radio_profile_17 = adf7024_radio_profile_regs[profile][17];
	ctx->conf_regs.radio_profile_18 = adf7024_radio_profile_regs[profile][18];
	ctx->conf_regs.radio_profile_19 = adf7024_radio_profile_regs[profile][19];
	ctx->conf_regs.radio_profile_20 = adf7024_radio_profile_regs[profile][20];
	ctx->conf_regs.radio_profile_21 = adf7024_radio_profile_regs[profile][21];
	ctx->conf_regs.radio_profile_22 = adf7024_radio_profile_regs[profile][22];

	// if (ctx->conf_regs.radio_afc_mode == ADF702X_VAL_RADIO_AFC_MODE_LOCK) {
		// radio_profile_3;
		// radio_profile_6;
	// }

	// MOD scheme
	// if (ctx->conf_regs.radio_afc_mode == ADF702X_VAL_RADIO_AFC_MODE_LOCK) {
	// just GFSK for now, set bit[3]
	ctx->conf_regs.radio_profile_7 |= 0x08;

	return 0;
};
#endif
