#include "./acta.h"
#include "./frda.h"
#include "./sheet.h"
#include "./sheet_t3x.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <citro2d.h>

#define handleFinish()                                   \
  needsReboot = true;                                    \
  if (R_SUCCEEDED(rc)) {                                 \
    printf("Success!\n");                                \
    printf("Changes will take affect after a reboot\n"); \
    printf("Press \"Start\" to reboot\n\n");             \
  }

#define handleResult(action, name)           \
  rc = action;                               \
  if (R_FAILED(rc)) {                        \
    printf("%s error: %08lx\n\n", name, rc); \
  }

/**
 * Switches the friends and act accounts.
 *
 * This will also create the act account if it's missing, just to be safe.
 * This will fail if the friend account hasn't been created yet.
 */
Result switchAccounts(u32 friend_account_id) {
  Result rc = 0;

  handleResult(FRDA_SetLocalAccountId(friend_account_id), "Switch account");
  if(rc){
    return rc;
  }

  u32 act_account_index = 0;
  handleResult(ACTA_GetAccountIndexOfFriendAccountId(&act_account_index, friend_account_id), "Get persistent id for creation");

  if (act_account_index == 0) {
    handleResult(ACTA_CreateLocalAccount(), "Create act account");
    handleResult(ACTA_GetAccountIndexOfFriendAccountId(&act_account_index, friend_account_id),
                 "Get persistent id after creation");
  }

  handleResult(ACTA_SetDefaultAccount(act_account_index), "Set default account");

  return rc;
}

Result createAccount() {
  Result rc = 0;

  // (Re)Create the friend account
  handleResult(FRDA_CreateLocalAccount(2, NASC_ENV_Test, 0, 1), "Create account");
  // Switch to the friend/act accounts
  handleResult(switchAccounts(2), "Switch account");
  // Reset the act account
  handleResult(ACTA_ResetAccount(2, true), "Reset account");

  return rc;
}

C2D_Sprite debug_button;
C2D_Sprite debug_header;
C2D_Sprite go_back;
C2D_Sprite header;
C2D_Sprite nintendo_deselected;
C2D_Sprite nintendo_btn_selected;
C2D_Sprite nintendo_selected;
C2D_Sprite nintendo_loaded_deselected;
C2D_Sprite pretendo_deselected;
C2D_Sprite pretendo_btn_selected;
C2D_Sprite pretendo_selected;
C2D_Sprite pretendo_loaded_deselected;
C2D_Sprite top;

C2D_TextBuf g_staticBuf;
C2D_TextBuf g_dynamicBuf;

u32 current_persistent_id = 0;
u32 screen = 0;

u32 act_account_count = 0;
u32 current_account = 0;

static void sceneInit(void)
{
  C2D_SpriteSheet spriteSheet = C2D_SpriteSheetLoadFromMem(sheet_t3x, sheet_t3x_size);
	C2D_SpriteFromSheet(&top, spriteSheet, sheet_top_idx);
	C2D_SpriteFromSheet(&debug_button, spriteSheet, sheet_debug_button_idx);
	C2D_SpriteFromSheet(&debug_header, spriteSheet, sheet_debug_header_idx);
	C2D_SpriteFromSheet(&go_back, spriteSheet, sheet_go_back_idx);
	C2D_SpriteFromSheet(&header, spriteSheet, sheet_header_idx);
	C2D_SpriteFromSheet(&nintendo_deselected, spriteSheet, sheet_nintendo_deselected_idx);
	C2D_SpriteFromSheet(&nintendo_btn_selected, spriteSheet, sheet_nintendo_btn_selected_idx);
	C2D_SpriteFromSheet(&nintendo_selected, spriteSheet, sheet_nintendo_selected_idx);
	C2D_SpriteFromSheet(&nintendo_loaded_deselected, spriteSheet, sheet_nintendo_loaded_deselected_idx);
	C2D_SpriteFromSheet(&pretendo_deselected, spriteSheet, sheet_pretendo_deselected_idx);
	C2D_SpriteFromSheet(&pretendo_btn_selected, spriteSheet, sheet_pretendo_btn_selected_idx);
	C2D_SpriteFromSheet(&pretendo_selected, spriteSheet, sheet_pretendo_selected_idx);
	C2D_SpriteFromSheet(&pretendo_loaded_deselected, spriteSheet, sheet_pretendo_loaded_deselected_idx);
	C2D_SpriteSetCenter(&top, 0.5f, 0.5f);
	C2D_SpriteSetPos(&top, 400/2, 240/2);
	C2D_SpriteSetPos(&debug_button, 107, 192);
	C2D_SpriteSetPos(&debug_header, 27, 22);
	C2D_SpriteSetPos(&go_back, 0, 214);
	C2D_SpriteSetPos(&header, 95, 14);
	C2D_SpriteSetPos(&pretendo_selected, 49, 59);
	C2D_SpriteSetPos(&pretendo_btn_selected, 49, 59);
	C2D_SpriteSetPos(&pretendo_deselected, 49, 59);
	C2D_SpriteSetPos(&pretendo_loaded_deselected, 49, 59);
	C2D_SpriteSetPos(&nintendo_selected, 165, 59);
	C2D_SpriteSetPos(&nintendo_btn_selected, 165, 59);
	C2D_SpriteSetPos(&nintendo_deselected, 165, 59);
	C2D_SpriteSetPos(&nintendo_loaded_deselected, 165, 59);
}

static void sceneRenderTop()
{
	C2D_DrawSprite(&top);
}

static void sceneRenderBottom()
{
  if(screen == 0){
    if(current_account == 0 || act_account_count == 1){
		  C2D_DrawSprite(&nintendo_selected);
		  C2D_DrawSprite(&pretendo_deselected);
    }else{
		  C2D_DrawSprite(&nintendo_deselected);
		  C2D_DrawSprite(&pretendo_selected);
    }
		C2D_DrawSprite(&header);
		//C2D_DrawSprite(&debug_button);
  }/*else{
		C2D_DrawSprite(&debug_header);
		C2D_DrawSprite(&go_back);
  }*/
}

static void sceneExit(void)
{
}

bool needsReboot = false;
int main()
{
	// Initialize the libs
  nsInit();
  frdAInit();
  actAInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

  u32 rc = 0;

  u32 act_account_index = 0;
  u32 current_persistent_id = 0;
  u32 pretendo_persistent_id = 0;
  handleResult(ACTA_GetAccountIndexOfFriendAccountId(&act_account_index, 2), "Get persistent id for Pretendo");
  handleResult(ACTA_GetPersistentId(&current_persistent_id, ACT_CURRENT_ACCOUNT), "Current persistent id");
  handleResult(ACTA_GetPersistentId(&pretendo_persistent_id, act_account_index), "Current persistent id");

  if(current_persistent_id == pretendo_persistent_id){
    current_account = 1;
  }else{
    current_account = 0;
  }

  // This version or higher is required creating/swapping friend accounts
  FRDA_SetClientSdkVersion(0x70000c8);

	// Create screen
  C3D_RenderTarget* top_screen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget* bottom_screen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Initialize the scene
	sceneInit();

  ACTA_GetAccountCount(&act_account_count);

  u32 btn_selected = 0;

	// Main loop
	while (aptMainLoop())
	{
    hidScanInput();
    touchPosition touch;
    hidTouchRead(&touch);
    u32 kDown = hidKeysDown();
    if (kDown & KEY_START) break;

	if (kDown & KEY_TOUCH) {
		btn_selected = 0;

		if ((touch.px >= 165 && touch.px <= 165 + 104) && (touch.py >= 59 && touch.py <= 59 + 113)) {
			switchAccounts(1);
			needsReboot = true;
			break;
		}
		else if ((touch.px >= 49 && touch.px <= 49 + 104) && (touch.py >= 59 && touch.py <= 59 + 113)) {
			if (switchAccounts(2)) {
				createAccount(2);
			}
			needsReboot = true;
			break;
		}
	}
	else if (kDown & KEY_LEFT)
	{
		if (btn_selected == 0) {
			btn_selected = 1;
		}
	
		btn_selected--;

		if (btn_selected < 1) {
			btn_selected = 2;
		}
	}
		
	else if (kDown & KEY_RIGHT)
	{
		if (btn_selected == 0) {
			btn_selected = 2;
		}

		btn_selected++;

		if (btn_selected > 2) {
			btn_selected = 1;
		}
	}

		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top_screen, C2D_Color32(21, 22, 28, 0xFF));
		C2D_SceneBegin(top_screen);
		sceneRenderTop();
		C2D_TargetClear(bottom_screen, C2D_Color32(21, 22, 28, 0xFF));
		C2D_SceneBegin(bottom_screen);
		if (btn_selected == 0) {
			sceneRenderBottom();
		}
		else if (btn_selected == 1)
		{
			if (current_account == 1)
			{
				C2D_DrawSprite(&nintendo_deselected);
				C2D_DrawSprite(&pretendo_selected);
			}
			else
			{
				C2D_DrawSprite(&nintendo_loaded_deselected);
				C2D_DrawSprite(&pretendo_btn_selected);
			}

			C2D_DrawSprite(&header);

			if (kDown & KEY_A)
			{
				if (switchAccounts(2)) {
					createAccount(2);
				}
				needsReboot = true;
				break;
			}
		}
		else if (btn_selected == 2)
		{
			if (current_account == 0)
			{
				C2D_DrawSprite(&nintendo_selected);
				C2D_DrawSprite(&pretendo_deselected);
			}
			else
			{
				C2D_DrawSprite(&nintendo_btn_selected);
				C2D_DrawSprite(&pretendo_loaded_deselected);
			}

			C2D_DrawSprite(&header);

			if (kDown & KEY_A)
			{
				switchAccounts(1);
				needsReboot = true;
				break;
			}
		}
		C3D_FrameEnd(0);
	}

	// Deinitialize the scene
	sceneExit();

	// Deinitialize the libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
  if(needsReboot){
    NS_RebootSystem();
  }
	return 0;
}
