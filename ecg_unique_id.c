#include "sapphire_pub.h"
#include "device_api.h"
#include "ecg_unique_id.h"

void get_unique_id(CardeaCmdGetUniqueIdResponse_t* response)
{
	uint8_t* id_cursor = (uint8_t*)ECG_UNIQUE_ID_REGISTER_ADDRESS;

	for(int index = 0;index < ECG_UNIQUE_ID_LENGTH;index++)
	{
		response->id[index] = *id_cursor;
		id_cursor++;
	}
}
