# Load application binary
file gcc/cardea_app_usb.axf

# Connect to OpenOCD
target extended-remote localhost:3333

# Configure Cortex-M3 breakpoint limits
set remote hardware-breakpoint-limit 6
set remote hardware-watchpoint-limit 4

# Disable IRQs while stepping
#define hook-step
#mon cortex_m maskisr on
#end
#define hookpost-step
#mon cortex_m maskisr off
#end

# Disable IRQs while nexting
#define hook-next
#mon cortex_m maskisr on
#end
#define hookpost-next
#mon cortex_m maskisr off
#end

# Disable IRQs while finishing
#define hook-finish
#mon cortex_m maskisr on
#end
#define hookpost-finish
#mon cortex_m maskisr off
#end

# Reset halt the device
#monitor reset halt

# Load the binary to the device
#load
