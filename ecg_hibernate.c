#include "driverlib/hibernate.h"

void ECG_Hibernate_Init()
{
	HibernateClockConfig(HIBERNATE_OSC_LFIOSC);
}
