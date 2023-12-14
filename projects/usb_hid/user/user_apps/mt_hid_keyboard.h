 0x05, 0x01, 		/* Usage Page (Desktop) */
        0x09, 0x06,         /* Keyboard */
        0xA1, 0x01, 		/* Collection (Application) */

            0x85, INPUT_REP_KEYBOARD_ID,	// 	Report Id 1
            0x05, 0x07, 						//	   Usage Page (Key Codes)

            0x95, 0x05, 						//	   Report Count (5)
            0x75, 0x01, 						//	   Report Size (1)
            0x05, 0x08, 						//	   Usage Page (Page# for LEDs)
            0x19, 0x01, 						//	   Usage Minimum (1)
            0x29, 0x05, 						//	   Usage Maximum (5)
            0x91, 0x02, 						//	   Output (Data, Variable, Absolute), Led report
            0x95, 0x01, 						//	   Report Count (1)
            0x75, 0x03, 						//	   Report Size (3)
            0x91, 0x01, 						//	   Output (Data, Variable, Absolute), Led report padding

            0x95, 0x06, 						//	   Report Count (6)
            0x75, 0x08, 						//	   Report Size (8)
            0x15, 0x00, 						//	   Logical Minimum (0)
            0x25, 0x65, 						//	   Logical Maximum (110)
            0x05, 0x07, 						//	   Usage Page (Key codes)
            0x19, 0x00, 						//	   Usage Minimum (0)
            0x29, 0x65, 						//	   Usage Maximum (110)
            0x81, 0x00, 						//	   Input (Data, Array) Key array(6 bytes)
        0xC0,			/* END_COLLECTION */