#include <libusb.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


#define USB_CLASS_AUDIO                1
#define USB_SUBCLASS_AUDIO_CONTROL     1
#define USB_SUBCLASS_AUDIO_STREAMING   2

#define USB_ENDPOINT_ISOCHRONOUS       0x01
#define USB_ENDPOINT_ASYNC             0x04
#define USB_ENDPOINT_ADAPTIVE          0x08

#define USB_CLASS_DESCRIPTOR           0x20
#define USB_INTERFACE_DESCRIPTOR_TYPE  0x04
#define USB_GENERAL_DESCRIPTOR_SUBTYPE 0x01
#define USB_FORMAT_DESCRIPTOR_SUBTYPE  0x02
#define USB_FORMAT_TYPE_I              0x01

#define USB_AUDIO_FEEDBACK_SIZE        3
#define USB_MAX_CONTROL_SIZE           64

#define USB_REQUEST_UAC_SET_CUR        0x01
#define USB_REQUEST_UAC_SET_MIN        0x02
#define USB_REQUEST_UAC_GET_CUR        0x81
#define USB_REQUEST_UAC_GET_MIN        0x82
#define USB_REQUEST_UAC_GET_MAX        0x83
#define USB_REQUEST_UAC_GET_RES        0x84
#define USB_UAC_VOLUME_SELECTOR        0x02
#define UAS_FREQ_CONTROL               0x01
#define UAS_PITCH_CONTROL              0x02
#define USB_REQUEST_DIR_MASK           0x80
#define UAC_TYPE_SPEAKER               0x0301
#define UAC_OUTPUT_TERMINAL            0x03
#define UAC_FEATURE_UNIT               0x06
#define UAC_VOLUME_CONTROL						 0x02
#define UAC_BASS_CONTROL							 0x03
#define UAC_TREBLE_CONTROL						 0x05
#define UAC_BASSBOOST_CONTROL					 0x09
#define UAC_AGC_CONTROL								 0x07



static struct libusb_device_handle *devh = NULL;

int16_t get_int16(int8_t requestType, int8_t request, int8_t channel) {
	int16_t retval = 0;
	libusb_control_transfer(devh,
 	LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE | 0x80,
	requestType, request << 8 | channel, 2<<8, 
	(unsigned char *)&retval, sizeof(int16_t), 1000);
	return retval;
	
}

int8_t get_int8(int8_t requestType, int8_t request, int8_t channel) {
	int8_t retval = 0;
	libusb_control_transfer(devh,
 	LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE | 0x80,
	requestType, request << 8 | channel, 2<<8, 
	(unsigned char *)&retval, sizeof(int8_t), 1000);
	return retval;
}

void set_int8(int8_t requestType, int8_t request, int8_t channel, int8_t value) {
	libusb_control_transfer(devh,
 	LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
	requestType, request << 8 | channel, 2<<8, 
	(unsigned char *)&value, sizeof(int8_t), 1000);
}

void set_int16(int8_t requestType, int8_t request, int8_t channel, int16_t value) {
	libusb_control_transfer(devh,
 	LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
	requestType, request << 8 | channel, 2<<8, 
	(unsigned char *)&value, sizeof(int16_t), 1000);
}

int main(int argc, char *argv[]) {
	

	
	libusb_context *ctx;
	if (libusb_init(&ctx) < 0)
		return -1;

	devh = libusb_open_device_with_vid_pid(ctx, 0x05fc, 0x7849);
	if (!devh) {
		fprintf(stderr, "Sound Sticks not found\n");
		goto error;
	}
	
	printf("Found Sound Sticks\n");
	
	if(argc == 3) {
		if(0 == strncmp(argv[1], "treble", 6)) {
			int8_t newValue = atoi(argv[2]);
			set_int8(USB_REQUEST_UAC_SET_CUR, UAC_TREBLE_CONTROL, 0, newValue);
		}
		if(0 == strncmp(argv[1], "bass", 4)) {
			int8_t newValue = atoi(argv[2]);
			set_int8(USB_REQUEST_UAC_SET_CUR, UAC_BASS_CONTROL, 0, newValue);
		}
		if(0 == strncmp(argv[1], "bassboost", 9)) {
			int8_t newValue = atoi(argv[2]);
			set_int8(USB_REQUEST_UAC_SET_CUR, UAC_BASSBOOST_CONTROL, 0, newValue);
		}
		if(0 == strncmp(argv[1], "agc", 3)) {
			int8_t newValue = atoi(argv[2]);
			set_int8(USB_REQUEST_UAC_SET_CUR, UAC_AGC_CONTROL, 0, newValue);
		}
		if(0 == strncmp(argv[1], "volume", 6)) {
			int16_t newValue = atoi(argv[2]);
			set_int16(USB_REQUEST_UAC_SET_CUR, UAC_VOLUME_CONTROL, 1, newValue);
			set_int16(USB_REQUEST_UAC_SET_CUR, UAC_VOLUME_CONTROL, 2, newValue);
		}
		
	}
				
	int16_t min_volume_left = get_int16(USB_REQUEST_UAC_GET_MIN, UAC_VOLUME_CONTROL, 1);
	int16_t max_volume_left = get_int16(USB_REQUEST_UAC_GET_MAX, UAC_VOLUME_CONTROL, 1);
	int16_t cur_volume_left = get_int16(USB_REQUEST_UAC_GET_CUR, UAC_VOLUME_CONTROL, 1);
	int16_t min_volume_right = get_int16(USB_REQUEST_UAC_GET_MIN, UAC_VOLUME_CONTROL, 2);
	int16_t max_volume_right = get_int16(USB_REQUEST_UAC_GET_MAX, UAC_VOLUME_CONTROL, 2);
	int16_t cur_volume_right = get_int16(USB_REQUEST_UAC_GET_CUR, UAC_VOLUME_CONTROL, 2);

	double left_db = cur_volume_left * 0.00390625;
	double right_db = cur_volume_right * 0.00390625;

	printf("VOLUME LEFT: min: %d, max: %d, cur: %d (%fdB)\n", min_volume_left, max_volume_left, cur_volume_left, left_db);
	printf("VOLUME RIGHT: min: %d, max: %d, cur: %d (%fdB)\n", min_volume_right, max_volume_right, cur_volume_right, right_db);


	int8_t min_bass = get_int8(USB_REQUEST_UAC_GET_MIN, UAC_BASS_CONTROL, 0);
	int8_t max_bass = get_int8(USB_REQUEST_UAC_GET_MAX, UAC_BASS_CONTROL, 0);
	int8_t cur_bass = get_int8(USB_REQUEST_UAC_GET_CUR, UAC_BASS_CONTROL, 0);

	double bass_db = cur_bass * 0.25;
	printf("BASS: min %d, max: %d, cur: %d (%fdB)\n", min_bass, max_bass, cur_bass, bass_db);



	int8_t min_treble = get_int8(USB_REQUEST_UAC_GET_MIN, UAC_TREBLE_CONTROL, 0);
	int8_t max_treble = get_int8(USB_REQUEST_UAC_GET_MAX, UAC_TREBLE_CONTROL, 0);
	int8_t cur_treble = get_int8(USB_REQUEST_UAC_GET_CUR, UAC_TREBLE_CONTROL, 0);



	double treble_db = cur_treble * 0.25;

	printf("TREBLE: min %d, max: %d, cur: %d (%fdB)\n", min_treble, max_treble, cur_treble, treble_db);

	int8_t bassboost = get_int8(USB_REQUEST_UAC_GET_CUR, UAC_BASSBOOST_CONTROL, 0);
	int8_t agc = get_int8(USB_REQUEST_UAC_GET_CUR, UAC_AGC_CONTROL, 0);

	if(bassboost > 0) {
		printf("Bass Boost: On, ");
	} else {
		printf("Bass Boost: Off, ");
	}

	if(agc > 0) {
		printf("Automatic Gain Control: On\n");
	} else {
		printf("Automatic Gain Control: Off\n");
	}


	libusb_exit(ctx);
	return 0;

error:

	libusb_exit(ctx);
  return -1;

}
