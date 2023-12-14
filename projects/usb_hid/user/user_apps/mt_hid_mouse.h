
0x05, 0x01, 									 // USAGE_PAGE (Generic Desktop)
0x09, 0x02, 									 // USAGE (Mouse)
0xa1, 0x01, 									 // COLLECTION (Application)
	0x85, INPUT_REP_MOUSE_ID,				           //		 Report Id
	0x09, 0x01, 									 // 	USAGE (Pointer)
	0xa1, 0x00, 									 // 	COLLECTION (Physical)
		0x05, 0x09, 									 // 		USAGE_PAGE (Button)
		0x19, 0x01, 									 // 		USAGE_MINIMUM (Button 1)
		0x29, 0x03, 									 // 		USAGE_MAXIMUM (Button 3)
		0x15, 0x00, 									 // 		LOGICAL_MINIMUM (0)
		0x25, 0x01, 									 // 		LOGICAL_MAXIMUM (1)
		0x95, 0x03, 									 // 		REPORT_COUNT (3)
		0x75, 0x01, 									 // 		REPORT_SIZE (1)
		
		0x81, 0x02, 									 // 		INPUT (Data,Var,Abs)
		0x95, 0x01, 									 // 		REPORT_COUNT (1)
		0x75, 0x05, 									 // 		REPORT_SIZE (5)
		0x81, 0x03, 									 // 		INPUT (Cnst,Var,Abs)
		
		0x05, 0x01, 									 // 		USAGE_PAGE (Generic Desktop)
		0x09, 0x30, 									 // 		USAGE (X)
		0x09, 0x31, 									 // 		USAGE (Y)
		0x15, 0x81, 									 // 		LOGICAL_MINIMUM (-127)
		0x25, 0x7f, 									 // 		LOGICAL_MAXIMUM (127)
		0x75, 0x08, 									 // 		REPORT_SIZE (8)
		0x95, 0x02, 									 // 		REPORT_COUNT (2)
		0x81, 0x06, 									 // 		INPUT (Data,Var,Rel)

    0x05, 0x01,                   // Usage Page (Generic Desktop)
    0x09, 0x38,                   // Usage (Wheel)
    0x15, 0x81,                   // Logical Minimum (-127)
    0x25, 0x7F,                   // Logical Maximum (127)
    0x75, 0x08,                   // Report Size (8)
    0x95, 0x01,                   // Report Count (1)    
    0x81, 0x06,                   // Input (Data, Variable, Relative)

	0xc0,												 // 	END_COLLECTION
0xc0, 

