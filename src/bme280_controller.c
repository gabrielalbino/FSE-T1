#include "bme280_controller.h"
struct bme280_dev dispositivo;
struct identificador user_id;

int BME280_setup(){
    int8_t rslt = BME280_OK;
    /* Variable to define the result */

    if ((user_id.descritorDoArquivo = open("/dev/i2c-1", O_RDWR)) < 0)
    {
        return rslt;
    }
    user_id.enderecoDoDispositivo = BME280_I2C_ADDR_PRIM;

    if (ioctl(user_id.descritorDoArquivo, I2C_SLAVE, user_id.enderecoDoDispositivo) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }


    dispositivo.intf = BME280_I2C_INTF;
    dispositivo.read = user_i2c_read;
    dispositivo.write = user_i2c_write;
    dispositivo.delay_us = user_delay_us;

    /* Update interface pointer with the structure that contains both device address and file descriptor */
    dispositivo.intf_ptr = &user_id;

    /* Initialize the bme280 */
    rslt = bme280_init(&dispositivo);
    if (rslt != BME280_OK)
    {
        fprintf(stderr, "Failed to initialize the device (code %+d).\n", rslt);
        exit(1);
    }
    return rslt;
}

double BME280_updateTemperature(volatile temperature* temperature){
int8_t rslt = BME280_OK;
  rslt = stream_sensor_data_normal_mode(&dispositivo, temperature);
  if (rslt != BME280_OK)
  {
      fprintf(stderr, "Failed to stream sensor data (code %+d).\n", rslt);
      exit(1);
  }
  return 0;
}

int8_t stream_sensor_data_normal_mode(struct bme280_dev *dev, volatile temperature* temperature)
{
	int8_t rslt;
	uint8_t settings_sel;
	struct bme280_data comp_data;

	/* Recommended mode of operation: Indoor navigation */
	dev->settings.osr_h = BME280_OVERSAMPLING_1X;
	dev->settings.osr_p = BME280_OVERSAMPLING_16X;
	dev->settings.osr_t = BME280_OVERSAMPLING_2X;
	dev->settings.filter = BME280_FILTER_COEFF_16;
	dev->settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

	settings_sel = BME280_OSR_PRESS_SEL;
	settings_sel |= BME280_OSR_TEMP_SEL;
	settings_sel |= BME280_OSR_HUM_SEL;
	settings_sel |= BME280_STANDBY_SEL;
	settings_sel |= BME280_FILTER_SEL;
	rslt = bme280_set_sensor_settings(settings_sel, dev);
	rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, dev);

		dev->delay_us(1000000, dev->intf_ptr);
		rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
		handle_sensor_data(&comp_data, temperature);

	return rslt;
}

void handle_sensor_data(struct bme280_data *comp_data,volatile temperature* temperature)
{
    temperature->out = comp_data->temperature;
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr)
{
    uint8_t *buf;
    struct identificador id;

    id = *((struct identificador *)intf_ptr);

    buf = malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    if (write(id.descritorDoArquivo, buf, len + 1) < (uint16_t)len)
    {
        return BME280_E_COMM_FAIL;
    }

    free(buf);

    return BME280_OK;
}



/*!
 * @brief This function reading the sensor's registers through I2C bus.
 */
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr)
{
    struct identificador id;

    id = *((struct identificador *)intf_ptr);
    write(id.descritorDoArquivo, &reg_addr, 1);
    read(id.descritorDoArquivo, data, len);
    return 0;
}


void user_delay_us(uint32_t period, void *intf_ptr)
{
    usleep(period);
}