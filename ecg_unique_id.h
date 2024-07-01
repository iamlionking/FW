#define ECG_UNIQUE_ID_REGISTER_ADDRESS	0x400FEF20
#define ECG_UNIQUE_ID_LENGTH			16

void get_unique_id(CardeaCmdGetUniqueIdResponse_t* response);
