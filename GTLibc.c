/*GTLibc - Library to make game trainer in c/c++.*/

#include "GTLibc.h"

/*Global variables for storing game information*/
DWORD gt_process_id = NIL;
HANDLE gt_game_handle = (HANDLE)NULL;
CHAR gt_game_name[MAX_PATH] = { NUL };
HWND gt_game_hwnd = (HWND)NULL;

/*Global variable for storing error code*/
DWORD gt_error_code = NIL;

/*Setting private methods inaccessible*/
BOOL gt_private_method = FALSE;

/*Setting add Logs to disable by default.*/
BOOL gt_logs_enabled = FALSE;

/*Store amount of nops*/
UINT n_nops = NIL;

/****************************************************************************/
/*********************-PUBLIC-METHODS-***************************************/
/****************************************************************************/

/**
 * @description - Find game by process name.
 * @param - Game name in string format (CASE INSENSTIVE). and without '.exe' extension.
 * @return - If game found it returns HANDLE to game otherwise returns NULL
 * NOTE : Process name is name of your .exe file loaded in Memory.
 */

HANDLE GT_FindGameProcess(LPCSTR game_name)
{
	gt_private_method = TRUE;
	SetLastError(NO_ERROR);

	UINT index = NIL, game_len = NIL, sz_exe_len = NIL, game_exe_len = lstrlen(game_name);
	LPSTR game_name_exe = NULL, game_exe = NULL;

	//process info variables.
	CHAR p_name[MAX_PATH] = { '\0' };
	DWORD p_id = NIL;
	HANDLE p_handle = (HANDLE)NULL;
	HWND p_hwnd = (HWND)NULL;

	LPSTR sz_exe_file = NULL;

	PROCESSENTRY32 entry;
	HANDLE snapshot = NULL;
	BOOL game_found = FALSE;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> game_name input : %s\n", FUNC_NAME, game_name);
		gt_private_method = TRUE;
	}

	gt_try
	{
		entry.dwSize = sizeof(PROCESSENTRY32);

		snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NIL);
		game_found = FALSE;
		gt_error_code = NIL;

		//allocate enough memory to store game name + ".exe" extension.
		game_exe = (LPSTR)GT_MemAlloc(HEAP_ZERO_MEMORY, game_exe_len + 5);

		//append ".exe" extension to match with szExeFile.
		lstrcpy(game_exe, game_name);
		lstrcat(game_exe, ".exe");

		gt_private_method = TRUE;
		//Main loop for iterating input game in process list.
		if (Process32First(snapshot, &entry))
		{
			while (Process32Next(snapshot, &entry))
			{
				if (!lstrcmpi(entry.szExeFile, game_exe))
				{
					game_found = TRUE;
					game_len = lstrlen(entry.szExeFile);
					game_name_exe = (LPSTR)GT_MemAlloc(HEAP_ZERO_MEMORY, game_len + 1);
					gt_private_method = TRUE;

					sz_exe_len = lstrlen(entry.szExeFile);
					sz_exe_file = (LPSTR)GT_MemAlloc(HEAP_ZERO_MEMORY, sz_exe_len + 1);
					gt_private_method = TRUE;

					//copy exe file name.
					lstrcpy(sz_exe_file,(LPCSTR)entry.szExeFile);

					//remove '.exe' part from game name.
					for (index = 0; index < lstrlen(sz_exe_file); index++)
					{
						if (sz_exe_file[index] == '.')
							break;

						game_name_exe[index] = sz_exe_file[index];
					}

					GT_MemFree((LPVOID)game_exe);
					gt_private_method = TRUE;

					GT_MemFree((LPVOID)sz_exe_file);
					gt_private_method = TRUE;
					break;
				}
			}

			//if game found.
			if (game_found)
			{
				//copy current process info.
				lstrcpy(p_name, game_name_exe);
				p_id = entry.th32ProcessID;
				p_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

				//set current process info.
				GT_SetGameName(p_name);
				GT_SetProcessID(p_id);
				GT_SetGameHandle(p_handle);
				GT_SetGameHWND(p_id);

				p_hwnd = GT_GetGameHWND();
				gt_private_method = TRUE;


				if (gt_logs_enabled)
				{
					GT_AddLog("Game \tname %s\tid : %u\tHandle : %p\thwnd : %p\n", p_name, p_id, p_handle, p_hwnd);
					gt_private_method = TRUE;
				}

				GT_MemFree((LPVOID)game_name_exe);
				gt_private_method = TRUE;
			}

			else
			{
				CloseHandle(snapshot);
				CloseHandle(gt_game_handle);

				GT_ShowError(ERROR_FILE_NOT_FOUND, FUNC_NAME, LINE_NO);
				gt_private_method = TRUE;
			}
		}
		else
		{
			CloseHandle(snapshot);
			CloseHandle(gt_game_handle);

			gt_private_method = TRUE;
			gt_throw(GT_GetError());
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  returned handle : %p\n", FUNC_NAME, gt_game_handle);
	}
	gt_private_method = FALSE;
	return p_handle;
}

/**
 * @description - Find game by window name.
 * @param - window name in string format (CASE INSENSITIVE).
 * @return - if game window found then it returns HWND to that window otherwise returns NULL
 * NOTE : Windows name is name of your Game Process Window not the .exe file.
 */

HWND GT_FindGameWindow(LPCSTR window_name)
{
	gt_private_method = TRUE;
	HWND game_window = NULL;
	gt_error_code = NIL;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> window_name input : %s\n", FUNC_NAME, window_name);
		gt_private_method = TRUE;
	}


	gt_try
	{
		game_window = FindWindowEx((HWND)NULL, (HWND)NULL, NUL, window_name);

		if (game_window == NULL || *window_name == NUL)
		{
			gt_throw(GT_GetError());
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned game_window : %p\n", FUNC_NAME, game_window);
	}
	gt_private_method = FALSE;
	return game_window;
}

/**
 * @description - Read value from provided address.
 * @param - Address in VOID* format.
 * @return - If read succeeded then it returns new value otherwise returns NIL.
 */

DWORD GT_ReadAddress(LPVOID address)
{
	HANDLE gt_game_handle = GT_GetGameHandle();
	DWORD dw_value = NIL;
	gt_error_code = NIL;
	gt_private_method = TRUE;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> trying to read from address %p\n", FUNC_NAME, address);
		gt_private_method = TRUE;
	}


	gt_try
	{
		if (gt_game_handle == NULL)
		{
				gt_throw(GT_GetError());
		}

		else
		{
			if (!ReadProcessMemory(gt_game_handle, address, &dw_value, sizeof(dw_value), NULL))
			{
				gt_error_code = GT_GetError();

				//if Error not set by ReadProcessMemory.
				if (gt_error_code == NO_ERROR)
				{
					gt_error_code = ERROR_PARTIAL_COPY;
					SetLastError(gt_error_code);
				}
				gt_throw(gt_error_code);
			}

		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned value : %u\n", FUNC_NAME, dw_value);
	}

	gt_private_method = FALSE;
	return dw_value;
}

/**
 * @description - Write value at provided address.
 * @param - Address in VOID* format and value to be written in DWORD format.
 * @return - If write is succeeded then it returns TRUE otherwise returns FALSE
 */

BOOL GT_WriteAddress(LPVOID address, DWORD value)
{

	HANDLE gt_game_handle = GT_GetGameHandle();
	BOOL write_status = FALSE;
	gt_error_code = NIL;
	gt_private_method = TRUE;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> Trying to write value %p at address %p\n", FUNC_NAME, value, address);
		gt_private_method = TRUE;
	}


	gt_try
	{
		if (gt_game_handle == NIL)
		{
			gt_throw(GT_GetError());
		}

		else
		{
			if (!(write_status = WriteProcessMemory(gt_game_handle, address, &value, sizeof(value), NULL)))
			{
				gt_error_code = GT_GetError();

				//if Error not set by ReadProcessMemory.
				if (gt_error_code == NO_ERROR)
				{
					gt_error_code = ERROR_NOACCESS;
					SetLastError(gt_error_code);
				}
				gt_throw(gt_error_code);
			}

		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned status : %d\n", FUNC_NAME, write_status);
	}

	gt_private_method = FALSE;
	return write_status;
}


/**
 * @description - Read value from provided address with offset.
 * @param - Address in VOID* format and offset in DWORD format.
 * @return - If read succeeded then it returns address of pointer otherwise returns NIL.
 */

DWORD GT_ReadAddressOffset(LPVOID lp_address, DWORD dw_offset)
{
	HANDLE gt_game_handle = GT_GetGameHandle();
	DWORD dw_value = NIL;

	LPDWORD lpdw_address = (LPDWORD)lp_address;
	DWORD dw_address = (DWORD)lpdw_address;
	gt_private_method = TRUE;
	gt_error_code = NIL;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> Trying to read value from address %p with offset %p\n", FUNC_NAME, lp_address, dw_offset);
		gt_private_method = TRUE;
	}


	gt_try
	{
		if (gt_game_handle == NIL)
		{
			gt_throw(GT_GetError());
		}

		else
		{
			lpdw_address = (LPDWORD)(dw_address + dw_offset);
			dw_value = GT_ReadAddress((LPVOID)lpdw_address);
			gt_private_method = TRUE;
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned value : %u\n", FUNC_NAME, dw_value);
	}

	gt_private_method = FALSE;
	return dw_value;
}

/**
 * @description - Write value at provided address with offset.
 * @param - Address in VOID* format and offset in DWORD format,value in DWORD format.
 * @return - If write succeeded then it returns TRUE otherwise returns FALSE.
 */

BOOL GT_WriteAddressOffset(LPVOID lp_address, DWORD dw_offset, DWORD dw_value)
{
	HANDLE gt_game_handle = GT_GetGameHandle();
	BOOL write_status = FALSE;
	gt_error_code = NIL;
	gt_private_method = TRUE;


	LPDWORD lpdw_address = (LPDWORD)lp_address;
	DWORD dw_address = (DWORD)lpdw_address;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> Trying to write value %u at address %p with offset %p\n", FUNC_NAME, dw_value, lp_address, dw_offset);
		gt_private_method = TRUE;
	}


	gt_try
	{
		if (gt_game_handle == NIL)
		{
			gt_throw(GT_GetError());
		}

		else
		{
			lpdw_address = (LPDWORD)(dw_address + dw_offset);
			write_status = GT_WriteAddress((LPVOID)lpdw_address, dw_value);
			gt_private_method = TRUE;


			if (!write_status)
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned status : %d\n", FUNC_NAME, write_status);
	}

	gt_private_method = FALSE;
	return write_status;
}

/**
 * @description - Read value from provided address with provided offsets.
 * @param - Address in VOID* format, offsets in DWORD* format and size of offsets.
 * @return - If read succeeded then it returns list of values otherwise returns NULL
 * NOTE : This will be useful in reading multiple values at a time like multiple Ammos/Clips from Ammos/Clips offsets list.
 * PS : FREE this memory after using it to avoid memory leaks use HeapFree() Method from (windows.h).
 */

DWORD* GT_ReadAddressOffsets(LPVOID lp_address, DWORD *dw_offsets, SIZE_T sz_offsets)
{
	HANDLE gt_game_handle = GT_GetGameHandle();
	UINT index = NIL;
	LPDWORD dw_values = NULL;
	gt_private_method = TRUE;
	gt_error_code = NIL;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> Trying to read values from address %p with offsets\n", FUNC_NAME, lp_address);
		gt_private_method = TRUE;
	}

	gt_try
	{
		dw_values = (LPDWORD)GT_MemAlloc(HEAP_ZERO_MEMORY, sz_offsets);
		gt_private_method = TRUE;

		if (gt_game_handle == NIL)
		{
			gt_throw(GT_GetError());
		}

		else if (dw_values == NULL)
		{
			if (gt_logs_enabled)
			{
				GT_AddLog("%s -> %s\n", FUNC_NAME, "offset_values is empty");
				gt_private_method = TRUE;
			}
			gt_throw(GT_GetError());
		}

		else
		{
			for (index = 0; index < sz_offsets / sizeof(DWORD); index++)
			{
				dw_values[index] = GT_ReadAddressOffset(lp_address, dw_offsets[index]);
				gt_private_method = TRUE;

				if (gt_logs_enabled)
				{
					GT_AddLog("Values[%u] : %p\tOffsets[%u] : %u\n", index, dw_values[index], index, dw_offsets[index]);
					gt_private_method = TRUE;
				}
			}
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned values : %p\n", FUNC_NAME, dw_values);
	}

	gt_private_method = FALSE;
	return dw_values;
}

/**
 * @description - Write value at provided address with provided offsets.
 * @param - Address in VOID* format and offset in DWORD* format, Size in SIZE_T format and value in DWORD format.
 * @return - If write succeeded then it returns TRUE otherwise returns FALSE.
  * NOTE : This will be useful in writing multiple values at a time like multiple ammo/clips values at Ammos/Clips offsets list.
 */

BOOL GT_WriteAddressOffsets(LPVOID lp_address, DWORD *dw_offsets, SIZE_T sz_offsets, DWORD dw_value)
{
	HANDLE gt_game_handle = GT_GetGameHandle();
	UINT index = NIL;
	BOOL write_status = FALSE;
	gt_error_code = NIL;
	gt_private_method = TRUE;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> Trying to write value %u at address %p with offsets\n", FUNC_NAME, dw_value, lp_address);
		gt_private_method = TRUE;
	}


	gt_try
	{
		if (gt_game_handle == NIL)
		{
			gt_private_method = TRUE;
			gt_throw(GT_GetError());
		}
		else
		{

			for (index = 0; index < sz_offsets / sizeof(DWORD); index++, dw_offsets++)
			{
				write_status = GT_WriteAddressOffset(lp_address, *dw_offsets, dw_value);
				gt_private_method = TRUE;

				if (gt_logs_enabled)
				{
					GT_AddLog("Offsets[%u] : %p\t with write status %d\n", index, dw_offsets[index], write_status);
					gt_private_method = TRUE;
				}


				if (!write_status)
				{
					gt_throw(GT_GetError());
				}
			}
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned status : %d\n", FUNC_NAME, write_status);
	}

	gt_private_method = FALSE;
	return write_status;
}


/**
 * @description - Read pointer's address from provided address with offset.
 * @param - Address in VOID* format and offset in DWORD format.
 * @return - If read succeeded then it returns address of pointer otherwise returns NULL.
 */

LPVOID GT_ReadPointerOffset(LPVOID lp_address, DWORD dw_offset)
{
	HANDLE gt_game_handle = GT_GetGameHandle();
	LPVOID lp_address_value = NULL;
	LPDWORD lpdw_address = (LPDWORD)lp_address;
	DWORD dw_address = (DWORD)lpdw_address;
	gt_private_method = TRUE;
	gt_error_code = NIL;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> Trying to read pointer from address %p with offset %p\n", FUNC_NAME, lp_address, dw_offset);
		gt_private_method = TRUE;
	}


	gt_try
	{
		if (gt_game_handle == NIL)
		{
			gt_throw(GT_GetError());
		}

		else
		{
			lpdw_address = (LPDWORD)(dw_address + dw_offset);
			lp_address_value = (LPVOID)GT_ReadAddress((LPVOID)lpdw_address);
			gt_private_method = TRUE;


			if (lp_address_value == NULL)
			{
			 gt_throw(GT_GetError());
			}
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned pointer : %p\n", FUNC_NAME, lp_address_value);
	}

	gt_private_method = FALSE;
	return lp_address_value;
}


/**
 * @description - Read pointer's address from provided address with provided offsets.
 * @param - Address in VOID* format, offsets in DWORD* format and size of offsets.
 * @return - If read succeeded then it returns address of pointer otherwise returns NULL.
 */

LPVOID GT_ReadPointerOffsets(LPVOID lp_address, DWORD *dw_offsets, SIZE_T sz_offsets)
{
	HANDLE gt_game_handle = NULL;
	UINT index = NIL;
	LPDWORD lpdw_address = NULL;
	gt_error_code = NIL;
	gt_private_method = TRUE;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  Trying to read pointer from address %p with offsets\n", FUNC_NAME, lp_address);
		gt_private_method = TRUE;
	}


	gt_try
	{
		gt_game_handle = GT_GetGameHandle();
		gt_private_method = TRUE;

		if (gt_game_handle == NIL)
		{
			gt_private_method = TRUE;
			gt_throw(GT_GetError());
		}
		else
		{
			lpdw_address = (LPDWORD)lp_address;

			for (index = 0; index < sz_offsets / sizeof(DWORD); index++)
			{
				lpdw_address = (LPDWORD)GT_ReadPointerOffset((LPVOID)lpdw_address, dw_offsets[index]);
				gt_private_method = TRUE;

				if (gt_logs_enabled)
				{
					GT_AddLog("offsets[%u] : %p read pointer address : %p\n", index, dw_offsets[index], lpdw_address);
					gt_private_method = TRUE;
				}

				if (lpdw_address == NULL)
				{
					gt_throw(GT_GetError());
				}
			}
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned pointer : %p\n", FUNC_NAME, (LPVOID)lpdw_address);
	}
	gt_private_method = FALSE;
	return (LPVOID)lpdw_address;
}

/**
 * @description - Write value at pointer's address with offset.
 * @param - Address in VOID* format and offset in DWORD format,value in DWORD format.
 * @return - If write succeeded then it returns TRUE otherwise returns FALSE.
 */

BOOL GT_WritePointerOffset(LPVOID lp_address, DWORD dw_offset, DWORD dw_value)
{
	HANDLE gt_game_handle = GT_GetGameHandle();
	DWORD dw_base_address_value = NIL, dw_real_address = NIL;
	BOOL write_status = FALSE;
	gt_error_code = NIL;
	gt_private_method = TRUE;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  Trying to write value %u at pointer %p with offset %p\n", FUNC_NAME, dw_value, lp_address, dw_offset);
		gt_private_method = TRUE;
	}


	gt_try
	{
		if (gt_game_handle == NIL)
		{
			gt_throw(GT_GetError());
		}

		dw_base_address_value = GT_ReadAddress(lp_address);
		gt_private_method = TRUE;

		if (dw_base_address_value == NIL)
		{
			gt_throw(GT_GetError());
		}

		else
		{
			dw_real_address = dw_base_address_value + dw_offset;
			write_status = GT_WriteAddress((LPVOID)dw_real_address, dw_value);
			gt_private_method = TRUE;


			if (!write_status)
			{
				gt_throw(GT_GetError());
			}
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  returned status : %d\n", FUNC_NAME, write_status);
	}

	gt_private_method = FALSE;
	return write_status;
}

/**
 * @description - Write value at pointer's address with offsets.
 * @param - Address in VOID* format and offset in DWORD* format,Size in SIZE_T format and value in DWORD format.
 * @return - If write succeeded then it returns TRUE otherwise returns FALSE.
 */

BOOL GT_WritePointerOffsets(LPVOID lp_address, DWORD *dw_offsets, SIZE_T sz_offsets, DWORD dw_value)
{
	HANDLE gt_game_handle = GT_GetGameHandle();
	UINT index = NIL;
	BOOL write_status = FALSE;
	gt_error_code = NIL;
	gt_private_method = TRUE;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  Trying to write value : %u at pointer %p with offsets\n", FUNC_NAME, dw_value, lp_address);
		gt_private_method = TRUE;
	}


	gt_try
	{
		if (gt_game_handle == NIL)
		{
			gt_throw(GT_GetError());
		}
		else
		{

			for (index = 0; index < sz_offsets / sizeof(DWORD); index++, dw_offsets++)
			{
				write_status = GT_WritePointerOffset(lp_address, *dw_offsets, dw_value);
				gt_private_method = TRUE;

				if (gt_logs_enabled)
				{
					GT_AddLog("Offsets[%u] : %u\twith write status : %d\n", index, dw_offsets[index], write_status);
					gt_private_method = TRUE;
				}



				if (!write_status)
				{
					gt_throw(GT_GetError());
				}
			}
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  returned status : %d\n", FUNC_NAME, write_status);
	}

	gt_private_method = FALSE;
	return write_status;
}

/**
 * @description -  Get current game name from memory.
 * @return - If game found returns game name otherwise returns NUL.
 */

LPCSTR GT_GetGameName()
{
	gt_private_method = TRUE;
	LPCSTR g_name = NUL;

	if ((gt_game_name[0] != NUL))
	{
		g_name = gt_game_name;
	}


	else
	{
		g_name = NUL;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  returned : %s\n", FUNC_NAME, g_name);
	}

	gt_private_method = FALSE;
	return g_name;
}

/**
 * @description -  Get process ID of game from memory.
 * @return - If game found returns process ID of current game otherwise returns NIL.
 */

DWORD GT_GetProcessID()
{
	gt_private_method = TRUE;
	DWORD p_id = NIL;

	if ((gt_process_id != NIL))
	{
		p_id = gt_process_id;
	}

	else
	{
		p_id = NIL;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  returned : %u\n", FUNC_NAME, p_id);
	}

	gt_private_method = FALSE;
	return p_id;
}

/**
 * @description - Get game handle from HWND (handle to window).
 * @param - Handle to current window of game in HWND format.
 * @return - Handle to process on success otherwise returns NULL.
 * NOTE : HANDLE is handle to Game's process and HWND is handle to Game's window.
 */

HANDLE GT_GetGameHandle4mHWND(HWND g_hwnd)
{
	gt_private_method = TRUE;
	DWORD p_id = NIL;
	HANDLE g_handle = NULL;
	gt_error_code = NIL;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  input Handle :  %p\n", FUNC_NAME, g_hwnd);
		gt_private_method = TRUE;
	}


	gt_try
	{
		if (g_hwnd == NULL || g_hwnd == INVALID_HANDLE_VALUE)
		{
			gt_throw(GT_GetError());
		}

		else
		{
			p_id = GT_GetProcessID4mHWND(g_hwnd);

			if (p_id == NIL)
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}
			g_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, p_id);
		}

	//set new valid game handle from HWND.
	gt_private_method = TRUE;
	if (g_handle != NULL)
	{
		GT_SetGameHandle(g_handle);
	}

	else
	{
		gt_private_method = TRUE;
		gt_throw(GT_GetError());
	}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  returned handle :  %p\n", FUNC_NAME, g_handle);
	}

	gt_private_method = FALSE;
	return g_handle;
}

/**
 * @description - Get process ID of game from HWND (handle to window).
 * @param - Handle to current window of game in HWND format.
 * @return - On success it returns game's process ID otherwise returns NIL.
 * NOTE : HANDLE is handle to Game's process and HWND is handle to Game's window.
 */

DWORD GT_GetProcessID4mHWND(HWND g_hwnd)
{
	gt_private_method = TRUE;
	DWORD p_id = NIL;
	gt_error_code = NIL;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  input Handle :  %p\n", FUNC_NAME, g_hwnd);
		gt_private_method = TRUE;
	}

	gt_try
	{
		if (g_hwnd == NULL || g_hwnd == INVALID_HANDLE_VALUE)
		{
			gt_throw(GT_GetError());

		}

		else
			GetWindowThreadProcessId(g_hwnd, &p_id);
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  returned process id :  %u\n", FUNC_NAME, p_id);
	}

	gt_private_method = FALSE;
	return p_id;
}

/**
 * @description - Get base address of current game.
 * @param - Process ID in DWORD format.
 * @return - On success it returns base address of game in (LPBYTE) format otherwise returns NUL.
 */

LPBYTE GT_GetGameBaseAddress(DWORD gt_process_id)
{
	gt_private_method = TRUE;
	SetLastError(NO_ERROR);

	HANDLE snapshot = NULL;
	MODULEENTRY32 module;
	LPBYTE base_address = NUL;
	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, gt_process_id);
	gt_error_code = NIL;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  input process id :  %u\n", FUNC_NAME, gt_process_id);
		gt_private_method = TRUE;
	}


	gt_try
	{
		if (snapshot == INVALID_HANDLE_VALUE)
		{
			gt_error_code = GT_GetError();

			//If error was not set by CreateToolhelp32Snapshot.
			if (gt_error_code == NO_ERROR)
			{
				gt_error_code = ERROR_INVALID_PARAMETER;
				SetLastError(gt_error_code);
			}
			gt_throw(gt_error_code);
		}

		else
		{
			module.dwSize = sizeof(MODULEENTRY32);
			Module32First(snapshot, &module);
			CloseHandle(snapshot);
			base_address = module.modBaseAddr;
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  returned address :  %p\n", FUNC_NAME, base_address);
	}

	gt_private_method = FALSE;
	return base_address;
}

/**
 * INFO : Hot keys can be in combination of like GT_HotKeysDown(LCTRL,VK_F1) or GT_HotKeysDown(LSHIFT,'F')
 * @description - check for Hot keys combinations pressed or not.
 * @param - Combination of hot keys using virtual key_codes or characters A-Z,a-z.
 * @return - If keys pressed it will return TRUE otherwise returns FALSE.
 * PS : Don't use this method directly instead use 'hotKeysPressed' MACRO.
 */

BOOL GT_HotKeysDown(INT key, ...)
{
	short result;
	va_list list;

	result = GetAsyncKeyState(key);
	for (va_start(list, key); key; key = va_arg(list, INT))
		result &= GetAsyncKeyState(key);

	va_end(list);

	return ((result & 0x8000) != 0);
}

/**
 * @description - Check if provided key is pressed or not.
 * @param - virtual key_code in INT format. ex : (VK_SHIFT).
 * @return - If key is pressed it returns TRUE otherwise returns FALSE.
 * NOTE : This method must be in main game running loop or any continuous loop.
 */

BOOL GT_IsKeyPressed(CONST INT key)
{
	SHORT key_state = GetAsyncKeyState(key);
	BOOL is_down = key_state & 0x8000;
	return is_down;
}

/**
 * @description - Check if provided key is toggled or not.
 * @param - virtual key_code in INT format. ex : (VK_SHIFT).
 * @return - If key is toggled it returns TRUE otherwise returns FALSE.
 * NOTE : This method must be in main game running loop or any continuous loop.
 */

BOOL GT_IsKeyToggled(CONST INT key)
{
	SHORT key_state = GetAsyncKeyState(key);
	BOOL is_toggled = key_state & 0x1;
	return is_toggled;
}

/****************************************************************************/
/****************-SEMI-PRIVATE-METHODS-**************************************/
/****************************************************************************/

/**
 * INFO : Send Mouse input to current game.
 * @description - Send mouse input control to game.
 * @param - mouse code in INT format. ex : use these pre-defined macros FROM_LEFT_1ST_BUTTON_PRESSED or RIGHTMOST_BUTTON_PRESSED.
 * NOTE : This will be useful if you want to create some automated scripting for your game.
 */

VOID GT_DoMousePress(INT mouse_code)
{
	gt_private_method = TRUE;
	GT_DoVirtualKeyPress(INPUT_MOUSE, NIL, mouse_code);
	gt_private_method = FALSE;
}

/**
 * INFO : Send Keyboard input to current game.
 * @description - Send keyboard input control to game.
 * @param - Virtual key_code in INT format. ex : VK_CONTROL,VK_SHIFT. (see winuser.h for full list of Key codes.)
 * NOTE : This will be useful if you want to create some automated scripting for your game.
 */

VOID GT_DoKeyPress(INT key_code)
{
	gt_private_method = TRUE;
	GT_DoVirtualKeyPress(INPUT_KEYBOARD, key_code, NIL);
	gt_private_method = FALSE;
}

/**
 * @description - Apply provided cheat code to current game.
 * @param - cheat code in string format, (NULL terminated).
 */
VOID GT_SetCheatCode(LPCSTR cheat_code)
{
	gt_private_method = TRUE;
	UINT index = NIL;
	size_t cheat_len = lstrlen(cheat_code);
	gt_private_method = TRUE;

	gt_try
	{
		if (gt_logs_enabled)
		{
			GT_AddLog("%s  ->  input cheat : %s\n",FUNC_NAME,cheat_code);
			gt_private_method = TRUE;
		}

		LPSTR cheat_buf = (LPSTR)GT_MemAlloc(HEAP_ZERO_MEMORY, cheat_len + 1);
		if (cheat_buf == NULL)
		{
			gt_throw(GT_GetError());
		}

		CopyMemory(cheat_buf, cheat_code, sizeof(CHAR) * cheat_len + 1);

		//convert cheat code to upper case for better mapping of characters.
		LPCSTR lp_cheat_upper = CharUpper(cheat_buf);

		//Time delay before entering cheat.
		Sleep(200);

		//Press all the cheat keys from cheat code.
		for (index = 0; index < cheat_len; index++)
		{
			GT_DoKeyPress(lp_cheat_upper[index]);
		}

		//Free allocated memory for cheat.
		gt_private_method = TRUE;
		GT_MemFree(cheat_buf);
	}

		gt_catch(gt_error_code)
	{
		gt_private_method = TRUE;
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
	}

	gt_private_method = FALSE;
}

/**
 * INFO : Search any value in current offset i.e. (base_address + offset) for finding new heath/ammos/weapons in game.
 * @description -  Search value in offset area.
 * @param - base address of Ammo/health/clips etc in VOID*,offset limit in size_t,offset size and value for searching.
 * @return - If value found it returns its address and offset in formatted string otherwise returns NULL
 * NOTE : FREE this memory after using it to avoid memory leaks use HeapFree() Method.
 */

LPSTR GT_SearchOffsetArea(LPVOID offset_base_address, CONST size_t offset_limit, CONST size_t offset_size, DWORD search)
{
	gt_private_method = TRUE;
	INT value = 0, offset_index = 0, offset = 0, offset_len = (offset_limit / offset_size);
	DWORD offset_base = (DWORD)offset_base_address;
	DWORD offset_address = offset_base;
	INT search_list_len = offset_len * 0x40;
	gt_error_code = NIL;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  Trying to search value : %u from address %p with offset limit : %u and offset size : %u\n", FUNC_NAME, search, offset_base_address, offset_limit, offset_size);
		gt_private_method = TRUE;
	}


	DWORD dw_search_size = search_list_len * sizeof(CHAR);
	LPSTR search_list = (LPSTR)GT_MemAlloc(HEAP_ZERO_MEMORY, dw_search_size);
	gt_private_method = TRUE;

	gt_try
	{
		if (search_list == NULL)
		{
			gt_throw(GT_GetError());
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> Search list size is : %u\n", FUNC_NAME, sizeof(search_list));
		gt_private_method = TRUE;
	}

	//copy headline status to search list.
	wsprintf(search_list, "%s\n", "SearchOffsetArea status : ");

	for (offset_index = 0; offset_index < offset_len; offset_index++)
	{
		offset_address += offset_size;
		offset = (INT)(offset_address - offset_base);
		value = GT_ReadAddress((LPVOID)offset_address);
		gt_private_method = TRUE;


		//if value found then copy its offset address etc in formatted string.
		if (value == search)
			wsprintf(search_list + lstrlen(search_list), "Value : %d\tAddress : %p\tOffset : %p\n", value, offset_address, offset);
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s  ->  returned values : %p\n", FUNC_NAME, search_list);
	}

	gt_private_method = FALSE;
	return search_list;
}


/**
 * INFO : Inject your custom code into the game.
 * @description - Inject single assembly/opcode directly into target process.
 * @param - Address where to inject, opcode to inject ,length of opcode in bytes.
 * @return - On success of injection it returns TRUE otherwise returns FALSE.

 * WARNING : This is advanced stuff so be careful while injecting custom code it should be exact opcode
 * for target machine's architecture ex : (x86,x64,amd64) and length of opcode should be exact as defined by ISA
 * otherwise target process could result in SEGFAULT causing program to crash.
 */

BOOL GT_InjectCode(LPVOID lp_address, LPCVOID lp_opcode, SIZE_T sz_opcode_len)
{
	HANDLE gt_game_handle = GT_GetGameHandle();
	BOOL inject_status = FALSE;
	DWORD old_protection = NIL;
	gt_error_code = NIL;
	gt_private_method = TRUE;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> Trying to inject code at address %p with opcode : %p of length : %lu\n", FUNC_NAME, lp_address, *(PUCHAR)(lp_opcode), sz_opcode_len);
		gt_private_method = TRUE;
	}

	gt_try
	{
		if (gt_game_handle == NIL)
		{
			gt_throw(GT_GetError());
		}

		else
		{
			//change protection of (.text) segment to EXECUTE/READWRITE.
			VirtualProtect((LPVOID)lp_address, sz_opcode_len, PAGE_EXECUTE_READWRITE, &old_protection);

			SetLastError(NO_ERROR);
			if (!(inject_status = WriteProcessMemory(gt_game_handle, lp_address, lp_opcode, sz_opcode_len, NULL)))
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}
			gt_private_method = TRUE;
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned status : %d\n", FUNC_NAME, inject_status);
	}

	//revert back to original protection of (.text) segment to READ_ONLY.
	VirtualProtect((LPVOID)lp_address, sz_opcode_len, old_protection, &old_protection);
	gt_private_method = FALSE;
	return inject_status;
}

/**
 * INFO : Inject your multiple custom codes into the game.
 * @description - Inject multiple assembly/opcodes directly into target process.
 * @param - List of Address where to inject, List of opcodes to inject , List of length of opcode in bytes, Total no. of Addresses.
 * @return - On success of injection it returns TRUE otherwise returns FALSE.

 * WARNING : This is advanced stuff so be careful while injecting custom code it should be exact opcode
 * for target machine's architecture ex : (x86,x64,amd64) and length of opcode should be exact as defined by ISA
 * otherwise target process could result in SEGFAULT causing program to crash.
 */

BOOL GT_InjectCodes(LPVOID lp_addresses[], LPBYTE lp_opcode[], SIZE_T sz_opcode_lens[], SIZE_T n_addresses)
{
	UINT index = NIL;
	BOOL inject_status = FALSE;

	for (index = 0; index < n_addresses; index++)
	{
		inject_status = GT_InjectCode(lp_addresses[index], lp_opcode[index], sz_opcode_lens[index]);
	}
	return inject_status;
}

/**
 * INFO : Write NOP (No-Operation) code into the game.
 * @description - Inject single assembly instruction NOP directly into target process.
 * @param - Address where to inject,size of opcode currently present at that address in bytes.
 * @return - On success of writing NOP it returns TRUE otherwise returns FALSE.

 * PS - size of opcode is needed because it will write NOP of N-byte length
 * where 'N' is size of current instruction present at that address.
 */

BOOL GT_WriteNOP(LPVOID lp_address, SIZE_T sz_opcode_len)
{
	BYTE NOP = '\x90';
	BOOL fill_status = FALSE;
	HANDLE gt_game_handle = GT_GetGameHandle();
	DWORD old_protection = NIL;
	gt_private_method = TRUE;

	LPBYTE lp_opcode = (LPBYTE)GT_MemAlloc(HEAP_ZERO_MEMORY, sz_opcode_len);
	gt_private_method = TRUE;

	gt_try
	{
		if (lp_opcode == NULL)
		{
			gt_throw(GT_GetError());
		}

		if (gt_logs_enabled)
		{
			GT_AddLog("%s -> Trying to fill NOPS at address %p of length : %lu\n", FUNC_NAME, lp_address, sz_opcode_len);
			gt_private_method = TRUE;
		}

		//Fill opcode with NOP.
		FillMemory(lp_opcode, sz_opcode_len, NOP);

		if (gt_game_handle == NIL)
		{
			gt_throw(GT_GetError());
		}

		else
		{
			//change protection of (.text) segment to EXECUTE/READWRITE.
			VirtualProtect(lp_address, sz_opcode_len, PAGE_EXECUTE_READWRITE, &old_protection);

			//inject NOP into at provided address.
			if (!(fill_status = WriteProcessMemory(gt_game_handle, lp_address, lp_opcode, sz_opcode_len, NULL)))
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}
		}

		GT_MemFree(lp_opcode);
		gt_private_method = TRUE;
	}
		gt_catch(gt_error_code)
	{
		gt_private_method = TRUE;
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	//revert back to original protection of (.text) segment to READ_ONLY.
	VirtualProtect(lp_address, sz_opcode_len, old_protection, &old_protection);

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned status : %d\n", FUNC_NAME, fill_status);
	}

	gt_private_method = FALSE;
	return fill_status;
}

/**
 * INFO : Write multiple NOP (No-Operation) code into the game.
 * @description - Inject multiple assembly instruction NOP directly into target process.
 * @param - List of Address where to inject,List of size of opcode present at that address, in bytes. Total no. of address.
 * @return - On success of writing NOP it returns TRUE otherwise returns FALSE.

 * PS - size of opcode is needed because it will write NOP of N-byte length
 * where 'N' is size of current instruction present at that address.
 */

BOOL GT_WriteNOPs(LPVOID lp_addresses[], SIZE_T sz_opcode_lens[], SIZE_T n_addresses)
{
	UINT index = NIL;
	BOOL nop_status = FALSE;
	gt_private_method = TRUE;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> trying to write with total of %lu address\n", FUNC_NAME, n_addresses);
		gt_private_method = TRUE;
	}

	for (index = 0; index < n_addresses; index++)
	{
		nop_status = GT_WriteNOP(lp_addresses[index], sz_opcode_lens[index]);
		gt_private_method = TRUE;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned status : %d\n", FUNC_NAME, nop_status);
	}
	gt_private_method = FALSE;
	return nop_status;
}

/**
 * @description - Write the JMP or CALL assembly instruction at an address.

 * @param - source - Address where to set the jmp/call instruction,
 * destination - Address where you want to JMP or CALL.
 * opcode_type - Use ENUM opcode to provide opcode type values.
 * nops_amount - Amount of NOPs to fill after JMP or CALL.
 * @return - On success of writing it returns TRUE otherwise returns FALSE.
 */

BOOL GT_WriteJmpOrCall(LPVOID lp_source, LPVOID lp_destination, GT_OPCODE opcode_type, UINT nops_amount)
{
	BOOL write_status = FALSE;
	DWORD old_protection = NIL, relative_offset = 0x5;
	UINT nop_amount = NIL;
	HANDLE gt_game_handle = GT_GetGameHandle();
	BYTE jmp_call_opcode = NUL;
	SIZE_T opcode_size = NIL;
	UINT index = NIL;
	gt_private_method = TRUE;

	LPDWORD source = (LPDWORD)lp_source;
	LPDWORD lpd_destination = (LPDWORD)lp_destination;
	DWORD destination = (DWORD)lpd_destination;

	//get the opcode type.
	switch (opcode_type)
	{
	case GT_OPCODE_SHORT_JUMP:
	{
		jmp_call_opcode = '\xEB';
		opcode_size = 0x2;
		break;
	}

	case GT_OPCODE_NEAR_JUMP:
	{
		jmp_call_opcode = '\xE9';
		opcode_size = 0x5;
		break;
	}

	case GT_OPCODE_CALL:
	{
		jmp_call_opcode = '\xE8';
		opcode_size = 0x5;
		break;
	}
	}

	gt_try
	{
		if (gt_game_handle == NIL)
		{
			gt_throw(GT_GetError());
		}

		else
		{
			//inject jmp or call opcode.
			if (!(write_status = GT_InjectCode((LPVOID)source, (LPCVOID)&jmp_call_opcode, sizeof(BYTE))))
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}

			gt_private_method = TRUE;
			//get the relative address.
			destination = destination - (DWORD)source - relative_offset;

			//change protection of (.text) segment to EXECUTE/READWRITE.
			VirtualProtect((LPVOID)source, opcode_size, PAGE_EXECUTE_READWRITE, &old_protection);

			if (gt_logs_enabled)
			{
				GT_AddLog("%s -> Trying to write at address : %p\tinstruction : %p\tof length : %lu\n", FUNC_NAME, (LPVOID)((DWORD)source + 0x1), (LPCVOID)&destination, opcode_size - 1);
				gt_private_method = TRUE;
			}

			//write destination address right after jmp or call opcode.
			if (!(write_status = WriteProcessMemory(gt_game_handle, (LPVOID)((DWORD)source + 0x1), (LPCVOID)&destination, opcode_size - 1, NULL)))
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}

			//If JMP short selected then fill rest of bytes with NOP.
			if (opcode_type == GT_OPCODE_SHORT_JUMP)
			{
				nop_amount = relative_offset - opcode_size;

				write_status = GT_WriteNOP((LPVOID)((DWORD)source + 0x2), nops_amount);
				gt_private_method = TRUE;
			}

			//If NOPS enabled then write amount of NOPs after JMP/CALL instruction.
			if (nops_amount > 0)
			{
				for (index = 0; index < nops_amount; index++)
					write_status = GT_WriteNOP((LPVOID)((DWORD)source + opcode_size + index), 0x1);
				gt_private_method = TRUE;
			}
		}
	}
		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	//revert back to original protection of (.text) segment to READ_ONLY.
	VirtualProtect((LPVOID)source, opcode_size, old_protection, &old_protection);

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned status : %d\n", FUNC_NAME, write_status);
	}

	gt_private_method = FALSE;
	return write_status;
}

/**
 * INFO : Inject custom or original procedure into the game using shellcode method.
 * @description - Inject your assembly procedure directly into target process.
 * @param -
  lp_address - Address where to inject procedure. 	[Required]
  lp_proc - Pointer to procedure's opcodes,size of procedure in bytes. [Required]
  sz_proc - Size of procedure opcodes in bytes. [Required]
  nops_amount - amount of NOPs to fill after injecting procedure. [Optional] (Required if GT_SHELL is of type Patched)
  shell_type - Shellcode type original or patched (custom) , Use Enum 'GT_SHELL' to provide values. [Required]
  opcode_type - Type of opcode to call your injected shellcode with, Use Enum 'GT_OPCODE' to provide values. [Optional] (Required if GT_SHELL is of type Patched)

 * @return -
   if GT_SHELL is of type Patched then On success of injection it returns address where procedure was injected otherwise returns NULL
   if GT_SHELL is of type Original it returns TRUE on success of code injection otherwise returns FALSE.

 * WARNING : This is advanced stuff so be careful while injecting procedure it should be exact opcode
 * for target machine's architecture ex : (x86,x64,amd64) and length of opcode should be exact as defined by ISA
 * otherwise target process could result in SEGFAULT causing program to crash.
 *
 * NOTE : Amount of NOPs to fill in necessary after instruction because CALL/JMP instruction is 5 bytes but current instruction could be lower
 * or higher , so in order to avoid crashing we will fill NOPs in empty locations.
 * Calculate using any disassembler the amount of NOPs needed after instruction depending upon current architecture.
 */

LPVOID GT_InjectProc(LPVOID lp_address, LPCVOID lp_proc, SIZE_T sz_proc, UINT nops_amount, GT_SHELL shell_type, GT_OPCODE opcode_type)
{
	BOOL inject_status = FALSE;
	LPVOID shell_code_address = NULL;
	n_nops = nops_amount;
	gt_private_method = TRUE;

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> Injecting proc at address : %p\twith size : %lu\n", FUNC_NAME, lp_address, sz_proc);
		gt_private_method = TRUE;
	}

	/*If shellcode is of original then inject shellcode with NIL NOPs*/
	if (shell_type == GT_ORIGINAL_SHELL)
	{
		inject_status = (BOOL)GT_InjectShellCode(lp_address, lp_proc, sz_proc, GT_ORIGINAL_SHELL, (GT_OPCODE)NIL);
		gt_private_method = TRUE;
	}

	/*If shellcode is of patched/custom then inject shellcode with provided NOPs*/
	else if (shell_type == GT_PATCHED_SHELL)
	{
		shell_code_address = GT_InjectShellCode(lp_address, lp_proc, sz_proc, GT_PATCHED_SHELL, opcode_type);
		gt_private_method = TRUE;

		if (opcode_type == GT_OPCODE_CALL)
		{
			inject_status = GT_WriteJmpOrCall(lp_address, shell_code_address, GT_OPCODE_CALL, nops_amount);
			gt_private_method = TRUE;
		}

		/*No matter which jump was selected we will use NEAR_JUMP because you can only call shellcode with NEAR_JUMP*/
		else if (opcode_type == GT_OPCODE_NEAR_JUMP || opcode_type == GT_OPCODE_SHORT_JUMP)
		{
			inject_status = GT_WriteJmpOrCall(lp_address, shell_code_address, GT_OPCODE_NEAR_JUMP, nops_amount);
			gt_private_method = TRUE;
		}
	}

	if (shell_type == GT_ORIGINAL_SHELL)
	{
		if (gt_logs_enabled)
		{
			GT_AddLog("%s -> returned status : %d\n", FUNC_NAME, inject_status);
		}

		gt_private_method = FALSE;
		return (LPVOID)inject_status;
	}

	//else otherwise return shell address.
	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned address : %p\n", FUNC_NAME, shell_code_address);
	}
	gt_private_method = FALSE;
	return shell_code_address;
}

/**
 * INFO : Inject custom or original shellcode into the game.
 * @description - Inject your custom/original shellcode directly into target process (Virtual Memory).
 * @param -
   lp_origshell_address - Address of original shellcode. [Optional] (Required only if GT_SHELL is of type Original or if Opcode is of type JMP)
   lp_shellcode - Pointer to shellcode. 	   [Required]
   sz_shellcode - size of shellcode in bytes. [Required]
  shell_type - Shellcode type original or patched (custom) , Use Enum 'GT_SHELL' to provide values. [Required]
  opcode_type - Type of opcode to call your injected shellcode with, Use Enum 'GT_OPCODE' to provide values. [Optional] (Required if GT_SHELL is of type Patched)

 * @return -
   if GT_SHELL is of type Patched then On success of injection it returns address where shellcode was injected otherwise returns NULL
   if GT_SHELL is of type Original it returns TRUE on success of code injection otherwise returns FALSE.

 * WARNING : This is advanced stuff so be careful while injecting shellcode it should be exact opcode
 * for target machine's architecture ex : (x86,x64,amd64) and length of opcode should be exact as defined by ISA
 * otherwise target process could result in SEGFAULT causing program to crash.
 */

LPVOID GT_InjectShellCode(LPVOID lp_origshell_address, LPCVOID lp_shellcode, SIZE_T sz_shellcode, GT_SHELL shell_type, GT_OPCODE opcode_type)
{
	HANDLE gt_game_handle = NULL, h_thread_snap = NULL, h_thread = NULL, target_thread = NULL;
	LPVOID shell_code_address = NULL;
	DWORD tid = NIL;
	UINT index = NIL;
	PUCHAR lpu_shellcode = NULL, ret_opcode = NULL;
	THREADENTRY32 te32;
	CONTEXT context;
	BOOL inject_status = FALSE;
	DWORD relative_offset = 0x5, jmp_opcode_size = 0x5, destination_address = NIL;
	SIZE_T sz_shellcode_buf = sz_shellcode;
	gt_private_method = TRUE;

	if (gt_logs_enabled)
	{
		LPCSTR shell_type_str = ((shell_type == GT_ORIGINAL_SHELL) ? "ORIGINAL GT_SHELL" : (shell_type == GT_PATCHED_SHELL) ? "PATCHED GT_SHELL" : NUL);
		LPCSTR opcode_type_str = ((opcode_type == GT_OPCODE_CALL) ? "GT_OPCODE CALL" : (opcode_type == GT_OPCODE_NEAR_JUMP) ? "GT_OPCODE NEAR JUMP" : (opcode_type == GT_OPCODE_SHORT_JUMP) ? "GT_OPCODE SHORT JUMP" : NUL);

		GT_AddLog("%s -> Injecting %s\tat Original address : %p\twith %s\n", FUNC_NAME, shell_type_str, lp_origshell_address, opcode_type_str);
		gt_private_method = TRUE;
	}

	gt_try
	{
		gt_game_handle = GT_GetGameHandle();
		gt_private_method = TRUE;

		if (gt_game_handle == NULL)
		{
			gt_throw(GT_GetError());
		}

		// Takes a snapshot of all threads in the system,
		h_thread_snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NIL);

		if (h_thread_snap == INVALID_HANDLE_VALUE)
		{
			gt_throw(GT_GetError());
		}
		else
		{
			if (gt_logs_enabled)
			{
				GT_AddLog("%s -> CreateToolhelp32Snapshot success\n", FUNC_NAME);
				gt_private_method = TRUE;
			}
		}

		// Retrieves information about the first thread of any process encountered in a system snapshot.
		te32.dwSize = sizeof(THREADENTRY32);
		if (Thread32First(h_thread_snap, &te32) == FALSE)
		{
			gt_throw(GT_GetError());
		}

		else
		{
			if (gt_logs_enabled)
			{
				GT_AddLog("%s -> Thread32First success\n", FUNC_NAME);
				gt_private_method = TRUE;
			}
		}


		/*From threads list display info about each thread*/
		do
		{
			if (te32.th32OwnerProcessID == GetProcessId(gt_game_handle))
			{
				if (tid == 0)
					tid = te32.th32ThreadID;
				h_thread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
				if (h_thread == NULL)
				{
					gt_throw(GT_GetError());
				}
				else
				{
					SuspendThread(h_thread);
					CloseHandle(h_thread);

					if (gt_logs_enabled)
					{
						GT_AddLog("%s -> Suspend thread %p\n", FUNC_NAME, te32.th32ThreadID);
						gt_private_method = TRUE;
					}

				}
			}
		} while (Thread32Next(h_thread_snap, &te32));
		CloseHandle(h_thread_snap);


		//if shell is of type patched.
		if (shell_type == GT_PATCHED_SHELL)
		{
			// Get context of current thread.
			context.ContextFlags = CONTEXT_FULL;

			// Open targeted thread with context,suspend and query flags.
			target_thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, tid);
			if (target_thread == NULL)
			{
				gt_throw(GT_GetError());
			}
			else
			{
				if (gt_logs_enabled)
				{
					GT_AddLog("%s -> Target thread %p\n", FUNC_NAME, target_thread);
					gt_private_method = TRUE;
				}
			}


			// Get eip & esp addresses if opcode is CALL.
			if (opcode_type == GT_OPCODE_CALL)
			{
				if (!GetThreadContext(target_thread, &context))
				{
					gt_throw(GT_GetError());
				}
				else
				{
					if (gt_logs_enabled)
					{
						GT_AddLog("\t%s -> CONTEXT BEFORE : \n\tEIP : %p\n\tESP : %p\n\tEBP : %p\n", FUNC_NAME, context.Eip, context.Esp, context.Ebp);
						gt_private_method = TRUE;

						GT_AddLog("\tEAX : %p\n\tEBX : %p\n\tESI : %p\n\tEDI : %p\n", context.Eax, context.Ebx, context.Esi, context.Edi);
						gt_private_method = TRUE;
					}
				}


				/* Save eip, esp & ebp and Allocate 4 bytes on the top of the stack for the RET if opcode is type CALL*/
				context.Esp -= sizeof(UINT);
				if (!WriteProcessMemory(gt_game_handle, (LPVOID)context.Esp, (LPCVOID)&context.Eip, sizeof(UINT), NULL))
				{
					gt_private_method = TRUE;
					gt_throw(GT_GetError());
				}
				else
				{
					if (gt_logs_enabled)
					{
						GT_AddLog("\t%s -> CONTEXT INJECT : \n\tEIP : %p\n\tESP : %p\n\tEBP : %p\n", FUNC_NAME, context.Eip, context.Esp, context.Ebp);
						gt_private_method = TRUE;

						GT_AddLog("\tEAX : %p\n\tEBX : %p\n\tESI : %p\n\tEDI : %p\n", context.Eax, context.Ebx, context.Esi, context.Edi);
						gt_private_method = TRUE;
					}
				}

				//Extra space for RET in case of CALL opcode.
				sz_shellcode += 0x1;

				//Allocate memory to append return opcode.
				lpu_shellcode = (PUCHAR)GT_MemAlloc(HEAP_ZERO_MEMORY, sz_shellcode);
				CopyMemory(lpu_shellcode, lp_shellcode, sz_shellcode - 0x2);

				//Append return opcode to current shellcode.
				ret_opcode = (PUCHAR)"\xC3";
				CopyMemory(lpu_shellcode + (sz_shellcode - 0x1), ret_opcode, sizeof(UCHAR));
				gt_private_method = TRUE;
			}

			//if opcode is JMP then Jump back next instruction.
			else if (opcode_type == GT_OPCODE_NEAR_JUMP || opcode_type == GT_OPCODE_SHORT_JUMP)
			{
				//Extra space for JMP in case of JMP opcode.
				sz_shellcode += jmp_opcode_size;

				//Allocate memory to append JMP opcode.
				lpu_shellcode = (PUCHAR)GT_MemAlloc(HEAP_ZERO_MEMORY, sz_shellcode);
				CopyMemory(lpu_shellcode, lp_shellcode, sz_shellcode - jmp_opcode_size);
				gt_private_method = TRUE;
			}

			//point to new appended shellcode.
			lp_shellcode = lpu_shellcode;

			//Add shellcode to Logs
			if (gt_logs_enabled)
			{
				GT_AddLog("%s -> Shellcode:\n", FUNC_NAME);
				gt_private_method = TRUE;

				for (index = 0; index < sz_shellcode; index++)
				{
					GT_AddLog("%p\t", *(PUCHAR)(((DWORD)lp_shellcode + index)));
					gt_private_method = TRUE;
				}
			}

			// Allocate memory in the targeted process for our shellcode
			shell_code_address = VirtualAllocEx(gt_game_handle, NULL, sz_shellcode, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (shell_code_address == NULL)
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}
			else
			{
				if (gt_logs_enabled)
				{
					GT_AddLog("\n%s -> Allocating Virtual memory of %d bytes for shellcode\n", FUNC_NAME, sz_shellcode);
					gt_private_method = TRUE;
				}
			}

			// Write the shellcode into the targeted thread
			if (!WriteProcessMemory(gt_game_handle, shell_code_address, (LPCVOID)lp_shellcode, sz_shellcode, NULL))
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}
			else
			{
				if (gt_logs_enabled)
				{
					GT_AddLog("%s -> shellcode inject success\n", FUNC_NAME);
					gt_private_method = TRUE;
				}
			}

			/*No matter which jump was selected we will use NEAR_JUMP because you can only jump
			back to original code from Virtual memory with NEAR_JUMP*/
			if (opcode_type == GT_OPCODE_SHORT_JUMP || opcode_type == GT_OPCODE_NEAR_JUMP)
			{
				destination_address = (DWORD)lp_origshell_address;
				destination_address += (relative_offset + n_nops);

				if (gt_logs_enabled)
				{
					GT_AddLog("%s -> destination_address : %p\n", FUNC_NAME, (LPVOID)destination_address);
					gt_private_method = TRUE;
				}

				GT_WriteJmpOrCall((LPVOID)((DWORD)shell_code_address + sz_shellcode_buf), (LPVOID)destination_address, GT_OPCODE_NEAR_JUMP, NIL);
				gt_private_method = TRUE;
			}

			if (opcode_type == GT_OPCODE_CALL)
			{
				// Redirect eip to the shellcode address
				context.Eip = (DWORD)shell_code_address;

				if (gt_logs_enabled)
				{
					GT_AddLog("\t%s -> CONTEXT REDIRECT : \n\tEIP : %p\n\tESP : %p\n\tEBP : %p\n", FUNC_NAME, context.Eip, context.Esp, context.Ebp);
					gt_private_method = TRUE;

					GT_AddLog("\tEAX : %p\n\tEBX : %p\n\tESI : %p\n\tEDI : %p\n", context.Eax, context.Ebx, context.Esi, context.Edi);
					gt_private_method = TRUE;
				}


				if (!SetThreadContext(target_thread, &context))
				{
					gt_private_method = TRUE;
					gt_throw(GT_GetError());
				}
				else
				{
					if (gt_logs_enabled)
					{
						GT_AddLog("%s -> SetThreadContext success\n", FUNC_NAME);
						gt_private_method = TRUE;
					}
				}
			}

		}


		else if (shell_type == GT_ORIGINAL_SHELL)
		{

			if (gt_logs_enabled)
			{
				GT_AddLog("%s -> Original shellcode\n", FUNC_NAME);
				gt_private_method = TRUE;

				for (index = 0; index < sz_shellcode; index++)
				{
					GT_AddLog("%p\t", *(PUCHAR)(((DWORD)lp_shellcode + index)));
					gt_private_method = TRUE;
				}
			}

			if (lp_origshell_address == NULL)
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}
			else
			{
				if (gt_logs_enabled)
				{
					GT_AddLog("\n%s -> Injecting original shellcode of %d bytes\n", FUNC_NAME, sz_shellcode);
					gt_private_method = TRUE;
				}
			}

			// Write the original shellcode.
			if (!(inject_status = WriteProcessMemory(gt_game_handle, lp_origshell_address, (LPCVOID)lp_shellcode, sz_shellcode, NULL)))
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}
			else
			{
				if (gt_logs_enabled)
				{
					GT_AddLog("%s -> shellcode inject success\n", FUNC_NAME);
					gt_private_method = TRUE;
				}
			}
		}


		// Resume Thread and take snapshot of all threads in the system, 0 to current process
		h_thread_snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NIL);
		te32.dwSize = sizeof(THREADENTRY32);
		if (h_thread_snap == INVALID_HANDLE_VALUE)
		{
			gt_private_method = TRUE;
			gt_throw(GT_GetError());
		}
		else
		{
			if (gt_logs_enabled)
			{
				GT_AddLog("%s -> CreateToolhelp32Snapshot success\n", FUNC_NAME);
				gt_private_method = TRUE;
			}
		}

		// Retrieves info about the first thread of any process encountered in a system snapshot.
		if (Thread32First(h_thread_snap, &te32) == FALSE)
		{
			gt_private_method = TRUE;
			gt_throw(GT_GetError());
		}
		else
		{
			if (gt_logs_enabled)
			{
				GT_AddLog("%s -> Thread32First success\n", FUNC_NAME);
				gt_private_method = TRUE;
			}
		}


		//From Thread list display information about each thread.
		do
		{
			if (te32.th32OwnerProcessID == GetProcessId(gt_game_handle))
			{
				if (gt_logs_enabled)
				{
					GT_AddLog("\%s -> THREAD ID = %p\n", FUNC_NAME, te32.th32ThreadID);
					gt_private_method = TRUE;
				}

				h_thread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
				if (h_thread == NULL)
				{
					gt_private_method = TRUE;
					gt_throw(GT_GetError());
				}
				else
				{
					ResumeThread(h_thread);
					if (te32.th32ThreadID == tid)
						if (WaitForSingleObject(h_thread,1000) == WAIT_OBJECT_0)
							CloseHandle(h_thread);

					if (gt_logs_enabled)
					{
						GT_AddLog("%s -> Resume thread success\n", FUNC_NAME);
						gt_private_method = TRUE;
					}

				}
			}
		} while (Thread32Next(h_thread_snap, &te32));

		//Close handle an free memory.
		CloseHandle(h_thread_snap);
		gt_private_method = TRUE;

		GT_MemFree(lpu_shellcode);
		gt_private_method = TRUE;
	}

		gt_catch(gt_error_code)
	{
		GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		gt_private_method = TRUE;
	}

	if (shell_type == GT_ORIGINAL_SHELL)
	{
		if (gt_logs_enabled)
		{
			GT_AddLog("%s -> returned status : %d\n", FUNC_NAME, inject_status);
		}
		gt_private_method = FALSE;
		return (LPVOID)inject_status;
	}

	//else otherwise return shell address.
	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned address : %p\n", FUNC_NAME, shell_code_address);
	}
	gt_private_method = FALSE;
	return shell_code_address;
}




/**
 * INFO : Whether library should maintain logs internally (enable this if you want this feature).
 * @description - Enable logs in library.
 * @return - Returns TRUE if enabled is success otherwise returns FALSE.
 */

BOOL GT_EnableLogs(VOID)
{
	gt_private_method = TRUE;
	BOOL enable_status = FALSE;

	if (gt_logs_enabled == FALSE)
	{
		gt_logs_enabled = TRUE;
		enable_status = TRUE;
	}
	else
	{
		GT_ShowWarning("Logs already enabled!");
		enable_status = FALSE;
	}
	gt_private_method = FALSE;
	return enable_status;
}

/**
 * @description - Disable logs in library.
 * @return - Returns TRUE if disable is success otherwise returns FALSE.
 */

BOOL GT_DisableLogs(VOID)
{
	gt_private_method = TRUE;
	BOOL disable_status = FALSE;

	if (gt_logs_enabled == TRUE)
	{
		gt_logs_enabled = FALSE;
		disable_status = TRUE;
	}
	else
	{
		GT_ShowWarning("Logs already disabled!");
		disable_status = FALSE;
	}
	gt_private_method = FALSE;
	return disable_status;
}

/**
 * @description - Get Handle to current game's process.
 * @return - If game found it return Handle to current game's process otherwise returns NULL.
 */

HANDLE GT_GetGameHandle()
{
	gt_private_method = TRUE;
	HANDLE g_handle = NULL;

	if (gt_game_handle != NULL)
	{
		g_handle = gt_game_handle;
	}

	else
	{
		g_handle = NULL;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned handle : %p\n", FUNC_NAME, g_handle);
	}

	gt_private_method = FALSE;
	return g_handle;
}

/**
 * @description - Get Handle to current game's window.
 * @return - If game found it return Handle to current game's windows otherwise returns NULL.
 */

HWND GT_GetGameHWND()
{
	gt_private_method = TRUE;
	HWND g_hwnd = NULL;

	if (gt_game_hwnd != NULL)
	{
		g_hwnd = gt_game_hwnd;
	}


	else
	{
		g_hwnd = NULL;
	}

	if (gt_logs_enabled)
	{
		GT_AddLog("%s -> returned handle : %p\n", FUNC_NAME, g_hwnd);
	}

	gt_private_method = FALSE;
	return g_hwnd;
}

/****************************************************************************/
/****************-PRIVATE-METHODS-*******************************************/
/****************************************************************************/

static VOID GT_ShowError(DWORD gt_error_code, LPCSTR gt_func_name, DWORD gt_line_no)
{
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
	{
		CHAR err_msg_buf[0xFF] = { '\0' };
		CHAR sys_err_buf[0xFF] = { '\0' };
		LPSTR ptr = NUL;
		gt_private_method = TRUE;


		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, gt_error_code,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			sys_err_buf, sizeof(sys_err_buf), (va_list *)NULL);

		// Trim the end of the line and terminate it with a null
		ptr = sys_err_buf;
		while ((*ptr > 31) || (*ptr == 9))
			++ptr;

		do
		{
			*ptr-- = '\0';
		} while ((ptr >= sys_err_buf) && ((*ptr == '.') || (*ptr < 33)));

		//copy error from getLastError() to error buffer.
		wsprintf(err_msg_buf, "\nINFO : %s method failed!\nREASON : (%s)\nLINE. : occurred at line no. %d\n", gt_func_name, sys_err_buf, gt_line_no);

		//Show error from error buffer.
		MessageBox((HWND)NULL, err_msg_buf, "ERROR!", MB_ICONERROR);

		if (gt_logs_enabled)
		{
			GT_AddLog("Error occurred : %s\n", err_msg_buf);
			gt_private_method = TRUE;
		}
	}
	gt_private_method = FALSE;
}

static VOID GT_ShowInfo(LPCSTR info_message)
{
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
	{
		MessageBox((HWND)NULL, info_message, "INFO!", MB_ICONINFORMATION);
		if (gt_logs_enabled)
		{
			GT_AddLog("Information shown : %s\n", info_message);
		}
	}
}

static VOID GT_ShowWarning(LPCSTR warning_message)
{
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
	{
		MessageBox((HWND)NULL, warning_message, "WARNING!", MB_ICONWARNING);
		if (gt_logs_enabled)
		{
			GT_AddLog("Warning shown : %s\n", warning_message);
		}
	}
}

static DWORD GT_GetError(void)
{
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
	{
		if (GetLastError() != ERROR_SUCCESS)
		{
			gt_error_code = GetLastError();
		}
	}
	return gt_error_code;
}

/*Setters for Process id,name,handle.*/
static VOID GT_SetProcessID(DWORD p_id)
{
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
		gt_process_id = p_id;
}

static VOID GT_SetGameHandle(HANDLE g_handle)
{
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
		gt_game_handle = g_handle;
}

static VOID GT_SetGameName(LPCSTR g_name)
{
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
		lstrcpy(gt_game_name, g_name);
}

static VOID GT_SetGameHWND(DWORD gt_process_id)
{
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
		EnumWindows(GT_EnumAllWindows, (LPARAM)gt_process_id);
}

/*Custom logger to add logs for trainer.*/
static VOID GT_AddLog(LPCSTR format, ...)
{
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
	{
		gt_private_method = TRUE;
		gt_error_code = NIL;
		CHAR log_buf[0x400] = { NUL };
		static LPCSTR log_file_name = "GTLibc_logs.log";

		INT date_len = NIL;
		static BOOL date_adder = FALSE;
		static BOOL file_checker = FALSE;

		//Only write data at beginning of file.
		if (!date_adder)
		{
			date_len = wsprintf(log_buf, "\nLog created by GTLibc at : Date : %s\tTime : %s\n", CURR_DATE, CURR_TIME);
			date_adder = TRUE;
		}

		va_list va_alist;
		va_start(va_alist, format);
		wvsprintf(log_buf + date_len, format, va_alist);
		va_end(va_alist);

		static HANDLE file_handle = NULL;

		if (!file_checker)
		{
			if (GT_FileExist(log_file_name))
			{
				file_handle = CreateFile(log_file_name, FILE_APPEND_DATA, NIL, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			}

			else
			{
				file_handle = CreateFile(log_file_name, FILE_WRITE_DATA, NIL, (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			}
			file_checker = TRUE;
		}

		gt_try
		{
			if (file_handle == INVALID_HANDLE_VALUE)
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}

			DWORD data_size = NIL;
			if (!WriteFile(file_handle, (LPCVOID)log_buf, lstrlen(log_buf), &data_size, (LPOVERLAPPED)NULL))
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}
		}
			gt_catch(gt_error_code)
		{
			GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
			gt_private_method = TRUE;
		}
	}
	gt_private_method = FALSE;
}

static BOOL GT_FileExist(LPCSTR file_name)
{
	BOOL does_exist = FALSE;
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
	{
		OFSTRUCT offstr;
		HFILE hfile = OpenFile(file_name, &offstr, OF_EXIST);

		if (hfile == HFILE_ERROR)
			does_exist = FALSE;
		else
			does_exist = TRUE;

		CloseHandle((HANDLE)hfile);
	}
	return does_exist;
}

static VOID GT_DoVirtualKeyPress(INT input_type, INT key_code, INT mouse_code)
{
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
	{
		//input event.
		INPUT input;

		if (input_type == INPUT_KEYBOARD)
		{
			// Set up a generic keyboard event.
			input.type = INPUT_KEYBOARD;
			input.ki.wScan = NIL; // hardware scan code for key
			input.ki.time = NIL;
			input.ki.dwExtraInfo = NIL;

			// Set the key to be pressed
			input.ki.wVk = key_code; // virtual-key code
			input.ki.dwFlags = 0;    // 0 for key press
		}

		else if (input_type == INPUT_MOUSE)
		{
			input.type = INPUT_MOUSE;
			input.mi.dx = NIL;
			input.mi.dy = NIL;
			input.mi.mouseData = NIL;
			input.mi.dwExtraInfo = NIL;
			input.mi.time = NIL;
			input.mi.dwFlags = (mouse_code == 1) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
		}

		//Press Key/Mouse down
		SendInput(1, &input, sizeof(INPUT));
		Sleep(200);

		//Setting Release Keyboard key
		if (input_type == INPUT_KEYBOARD)
			input.ki.dwFlags = KEYEVENTF_KEYUP;

		//Setting Release Mouse key
		if (input_type == INPUT_MOUSE)
			input.mi.dwFlags = (mouse_code == 1) ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;

		//Release Key/Mouse down
		SendInput(1, &input, sizeof(INPUT));
		Sleep(200);
	}
}

static BOOL CALLBACK GT_EnumAllWindows(HWND p_hwnd, LPARAM p_id)
{
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
	{
		DWORD gt_process_id = NIL;
		GetWindowThreadProcessId(p_hwnd, &gt_process_id);
		if (gt_process_id == p_id)
		{
			gt_game_hwnd = p_hwnd;
			return FALSE;
		}
	}
	return TRUE;
}

/*Private memory allocation wrapper methods*/
LPVOID GT_MemAlloc(DWORD dw_flag, SIZE_T sz_size)
{
	LPVOID gt_mem_block = NULL;
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
	{
		gt_try
		{
			gt_mem_block = HeapAlloc(GetProcessHeap(), dw_flag, sz_size);

			if (gt_mem_block == NULL)
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}
			else
			{
				if (gt_logs_enabled)
				{
					gt_private_method = TRUE;
					GT_AddLog("%s -> successfully allocated %lu bytes\n", FUNC_NAME, sz_size);
				}
			}

		}
			gt_catch(gt_error_code)
		{
			gt_private_method = TRUE;
			GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		}
	}

	gt_private_method = FALSE;
	return gt_mem_block;
}

/*Private memory de-allocate wrapper methods*/
BOOL GT_MemFree(LPVOID gt_mem_block)
{
	BOOL free_status = FALSE;
	if (GT_IsPrivateMethod(gt_private_method, FUNC_NAME, LINE_NO))
	{
		gt_try
		{
			if (!(free_status = HeapFree(GetProcessHeap(), NIL, gt_mem_block)))
			{
				gt_private_method = TRUE;
				gt_throw(GT_GetError());
			}

			else
			{
				if (gt_logs_enabled)
				{
					gt_private_method = TRUE;
					GT_AddLog("%s -> successfully freed memory block\n", FUNC_NAME);
				}
			}
		}
			gt_catch(gt_error_code)
		{
			gt_private_method = TRUE;
			GT_ShowError(gt_error_code, FUNC_NAME, LINE_NO);
		}
	}
	gt_private_method = FALSE;
	return free_status;
}

static BOOL GT_IsPrivateMethod(BOOL gt_private_method, LPCSTR gt_func_name, INT gt_line_no)
{
	if (gt_private_method)
		return TRUE;

	else
	{
		CHAR err_buf[0x100] = { NUL };
		wsprintf(err_buf, "ERROR : %s method failed!\nREASON : Access to private method! (ERROR_INVALID_FUNCTION) \nLINE : occurred at line no. %d", gt_func_name, gt_line_no);
		MessageBox((HWND)NULL, err_buf, "ERROR!", MB_ICONERROR);

		DWORD exit_code = NIL;
		GetExitCodeProcess(GetCurrentProcess(), &exit_code);
		ExitProcess(exit_code);
	}
}