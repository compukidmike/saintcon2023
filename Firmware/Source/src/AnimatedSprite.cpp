#include "AnimatedSprite.h"
#include <Arduino_GFX_Library.h>

#include "gif.inl"

#include "assets/bosses/LadyRobot_gif.h"
#include "assets/bosses/Monitors_gif.h"
#include "assets/bosses/Planet_gif.h"
#include "assets/bosses/SheepSuit_gif.h"
#include "assets/bosses/Sheep_gif.h"
#include "assets/bosses/lockbot.h"
#include "assets/bosses/sentinelbot.h"
#include "assets/bosses/k33n.h"
#include "assets/bosses/pcap.h"
#include "assets/bosses/tcpzwindow.h"
#include "assets/bosses/dupack.h"
#include "assets/bosses/jason.h"
#include "assets/bosses/reppilk.h"
#include "assets/bosses/training_enemies.h"
#include "assets/bosses/incident3.h"
#include "assets/bosses/incident4.h"
#include "assets/bosses/incident5.h"
#include "assets/bosses/incident6.h"
#include "assets/bosses/incident7.h"
#include "assets/heros/avatar1.h"
#include "assets/heros/avatar2.h"
#include "assets/heros/avatar3.h"
#include "assets/heros/avatar4.h"
#include "assets/ui/committee.h"

#include "badge.h"

struct gif_cb {
    int startx, starty;
    int disolve;
};

const int drows[10] = {0,7,3,8,2,9,4,6,1,5};

extern Arduino_Canvas *gfx;

GIFIMAGE _gifs[MAX_GIFS];
bool    _gifs_used[MAX_GIFS] = {0};

void GIFDraw(GIFDRAW *pDraw) {
    int x = ((gif_cb*)pDraw->pUser)->startx + pDraw->iX;
    int y = ((gif_cb*)pDraw->pUser)->starty + pDraw->iY + pDraw->y;
    int disolve = ((gif_cb*)pDraw->pUser)->disolve;
    if ( disolve > 0) {
        for (int i=0; i< disolve; ++i)
            if ((y%10) == drows[i])
                return;
    }
    for (int i=0; i<pDraw->iWidth; ++i) {
        if (pDraw->pPixels[i] != pDraw->ucTransparent) {
            gfx->drawPixel(x+i, y, pDraw->pPalette[pDraw->pPixels[i]]);
        }
    }
}

AnimatedSprite::AnimatedSprite() {
    for (int i=0; i< MAX_GIFS; ++i) {
        if (_gifs_used[i] == false) {
            decoder_id = i;
            _gifs_used[decoder_id] = true;
            return;
        }
    }
    decoder_id = 0;
}
AnimatedSprite::~AnimatedSprite() {
    _gifs_used[decoder_id] = false;
}
    
void AnimatedSprite::SetGif(uint8_t *pData, int iDataSize) {
    memset(&_gifs[decoder_id], 0, sizeof(GIFIMAGE));
    _gifs[decoder_id].ucPaletteType = GIF_PALETTE_RGB565_LE;
    _gifs[decoder_id].ucDrawType = GIF_DRAW_RAW; // assume RAW pixel handling
    _gifs[decoder_id].pFrameBuffer = NULL;
    _gifs[decoder_id].iError = GIF_SUCCESS;
    _gifs[decoder_id].pfnRead = readMem;
    _gifs[decoder_id].pfnSeek = seekMem;
    _gifs[decoder_id].pfnDraw = GIFDraw;
    _gifs[decoder_id].pfnOpen = NULL;
    _gifs[decoder_id].pfnClose = NULL;
    _gifs[decoder_id].GIFFile.iSize = iDataSize;
    _gifs[decoder_id].GIFFile.pData = pData;
    GIFInit(&_gifs[decoder_id]);
}

void AnimatedSprite::SetGif(String name) {
    if (name.equals("Player")) {
        switch(g_badgeinfo.character){
            case 0:
                SetGif((uint8_t*)avatar1, AVATAR1_SIZE); break;
            case 1:
                SetGif((uint8_t*)avatar2, AVATAR2_SIZE); break;
            case 2:
                SetGif((uint8_t*)avatar3, AVATAR3_SIZE); break;
            case 3:
                SetGif((uint8_t*)avatar4, AVATAR4_SIZE); break;
        }
    }
    else if (name.equals("sheep")) SetGif((uint8_t*)Sheep, SHEEPBOSS_SIZE); 
    else if (name.equals("TrainingBot1")) SetGif((uint8_t*)TrainingBot1, TRAININGBOT1_SIZE);
    else if (name.equals("TrainingBot2")) SetGif((uint8_t*)TrainingBot2, TRAININGBOT2_SIZE);
    else if (name.equals("TrainingBot3")) SetGif((uint8_t*)TrainingBot3, TRAININGBOT3_SIZE); 
    else if (name.equals("Incident 1 Enemy 1")) SetGif((uint8_t*)TrainingBot1, TRAININGBOT1_SIZE);
    else if (name.equals("Incident 1 Enemy 2")) SetGif((uint8_t*)TrainingBot2, TRAININGBOT2_SIZE);
    else if (name.equals("Incident 1 Enemy 3")) SetGif((uint8_t*)TrainingBot3, TRAININGBOT3_SIZE); 
    else if (name.equals("SuperTrainingBot")) SetGif((uint8_t*)SuperTrainingBot, SUPERTRAININGBOT_SIZE);
    else if (name.equals("Lockbot")) SetGif((uint8_t*)lockbot, LOCKBOT_SIZE);
    else if (name.equals("SentinelBot")) SetGif((uint8_t*)sentinelbot, SENTINELBOT_SIZE);
    //else if (name.equals("K33n")) SetGif((uint8_t*)k33n, K33N_SIZE);
    else if (name.equals("k33n")) SetGif((uint8_t*)k33nl, KEENL_SIZE);
    else if (name.equals("PCAP")) SetGif((uint8_t*)pcap, PCAP_SIZE);
    else if (name.equals("TCPZWindow")) SetGif((uint8_t*)TCPZWindow, TCPZWINOW_SIZE);
    else if (name.equals("DupACK")) SetGif((uint8_t*)DupACK, DUPACK_SIZE);
    else if (name.equals("Jason")) SetGif((uint8_t*)jason, JASON_SIZE);
    else if (name.equals("Reppilk")) SetGif((uint8_t*)Reppilk, REPPILK_SIZE);
    else if (name.equals("SavvyCustomerServiceRep")) SetGif((uint8_t*)SavvyCustomerServiceRep, SAVVYCSR_SIZE); 
    else if (name.equals("BillTheFrontlineCSR")) SetGif((uint8_t*)BillTheFriontlineCSR, BILLCRS_SIZE);
    else if (name.equals("PaveTheSupervisor")) SetGif((uint8_t*)PaveTheSupervisor, PAVETHESUPER_SIZE);
    else if (name.equals("PhoneTreeDeadends")) SetGif((uint8_t*)PhoneTreeDeadends, PHONETREE_SIZE);
    else if (name.equals("WAF")) SetGif((uint8_t*)WAF, WAF_SIZE);
    else if (name.equals("IPSentry")) SetGif((uint8_t*)IPSentry, IPSENTRY_SIZE);
    else if (name.equals("OldCVE")) SetGif((uint8_t*)OldCVE, OLDVCVE_SIZE); 
    else if (name.equals("M'iynt")) SetGif((uint8_t*)M_iynt, MIYNT_SIZE);
    else if (name.equals("Kahala")) SetGif((uint8_t*)Kahala, KAHALA_SIZE);
    else if (name.equals("Dolly")) SetGif((uint8_t*)Dolly, DOLLY_SIZE);
    else if (name.equals("PinkMustache")) SetGif((uint8_t*)PinkMustache, PINKMUSTACHE_SIZE);
    else if (name.equals("Tenderfoot")) SetGif((uint8_t*)Tender, TENDER_SIZE);
    else if (name.equals("X-10")) SetGif((uint8_t*)X_10, X_10_SIZE);
    else if (name.equals("UncleOakley")) SetGif((uint8_t*)UncleOakley, UNCLEOAKLEY_SIZE);
    else if (name.equals("RockyCoinChanger")) SetGif((uint8_t*)RockyCoinChanger, ROCKYCC_SIZE);
    else if (name.equals("TheArchitect")) SetGif((uint8_t*)TheArchitect, THEARCHITECT_SIZE);
    else if (name.equals("DeprecatedSwitchSyntax")) SetGif((uint8_t*)DeprecatedSwitchSyntax, DEPRECATED_SIZE);
    else if (name.equals("DoorControl")) SetGif((uint8_t*)DoorControl, DOORCONTROL_SIZE);
    else if (name.equals("sheepsuit")) SetGif((uint8_t*)Sheep_Suit_Boss, SHEEPSUITBOSS_SIZE);
    else if (name.equals("monitors"))  SetGif((uint8_t*)Monitors_Boss, MONITORSBOSS_SIZE);
    else if (name.equals("planet"))   SetGif((uint8_t*)Planet_Boss, PLANETBOSS_SIZE); 
    else if (name.equals("BryceKunz"))  SetGif((uint8_t*)BryceKunz, BRYCEKUNZ_GIF);
    else if (name.equals("TweekFawkes"))  SetGif((uint8_t*)BryceKunz, BRYCEKUNZ_GIF);
    else if (name.equals("Berly"))  SetGif((uint8_t*)Berly, BERLY_SIZE);
    else if (name.equals("Chunk"))  SetGif((uint8_t*)Chunk, CHUNCK_SIZE);
    else if (name.equals("Compukidmike"))  SetGif((uint8_t*)Compukidmike, COMPUKIDMIKE_SIZE);
    else if (name.equals("honki"))  SetGif((uint8_t*)Honki, HONKI_SIZE);
    else if (name.equals("Jup1t3r"))  SetGif((uint8_t*)Jup1t3r, JUPITER_SIZE);
    else if (name.equals("Jupiter"))  SetGif((uint8_t*)Jup1t3r, JUPITER_SIZE);
    else if (name.equals("Jup1t3r Boss"))  SetGif((uint8_t*)Planet_Boss, PLANETBOSS_SIZE);
    else if (name.equals("Kampf"))  SetGif((uint8_t*)Kampf, KAMPF_SIZE);
    else if (name.equals("Katie"))  SetGif((uint8_t*)Katie, KATIE_SIZE);
    else if (name.equals("Klippy"))  SetGif((uint8_t*)Klippy, KLIPPY_SIZE);
    else if (name.equals("Klipper"))  SetGif((uint8_t*)Kilpper, KLIPPER_SIZE);
    else if (name.equals("Pali"))  SetGif((uint8_t*)Pali, PALI_SIZE);
    else if (name.equals("Pope"))  SetGif((uint8_t*)Pope, POPE_SIZE);
    else if (name.equals("Ray-man"))  SetGif((uint8_t*)Rayman, RAYMAN_SIZE);
    else if (name.equals("Redactd"))  SetGif((uint8_t*)Redactd, REDACTED_SIZE);
    else if (name.equals("SJ"))  SetGif((uint8_t*)SJ, SJ_SIZE);
    else if (name.equals("Scr4m"))  SetGif((uint8_t*)Scr4m, SCRAM_SIZE);
    else if (name.equals("Sirashrum"))  SetGif((uint8_t*)Sirashrum, SIRASHRUM_SIZE);
    else if (name.equals("Supertechguy"))  SetGif((uint8_t*)Supertechguy, SUPERTECH_SIZE);
    else if (name.equals("Zevlag"))  SetGif((uint8_t*)Zevlag, ZEVLAG_SIZE);
    else if (name.equals("Zodiak"))  SetGif((uint8_t*)Zodiak, ZODIAK_SIZE);
    else if (name.equals("CrashCartWithABadWheel"))  SetGif((uint8_t*)CrashCartWithABadWheel, CRASHCART_SIZE);
    else if (name.equals("ForceFieldPadlock"))  SetGif((uint8_t*)ForceFieldPadlock, FORCELOCK_SIZE);
    else if (name.equals("SecureDoor"))  SetGif((uint8_t*)SecureDoor, SECUREDOOR_SIZE);
    else if (name.equals("SneakyNMAP"))  SetGif((uint8_t*)SneakyNMAP, NMAP_SIZE);
    else if (name.equals("SIEMAlerts"))  SetGif((uint8_t*)SIEMAlerts, SNORT_SIZE);
    else if (name.equals("PilesAndPilesOfRawLogData"))  SetGif((uint8_t*)Monitors_Boss, MONITORSBOSS_SIZE);
    else if (name.equals("MinibadgeSwarm"))  SetGif((uint8_t*)MinibadgeSwarm, MINISWARM_SIZE);
    else if (name.equals("DJDEFGoon"))  SetGif((uint8_t*)Kampf, KAMPF_SIZE);
    else if (name.equals("CentralizedIdentityProvider"))  SetGif((uint8_t*)CentralizedIdentityProvider, CENTRALIP_SIZE);
    else if (name.equals("RandomCeilingTile"))  SetGif((uint8_t*)RandomCeilingTile, RANDOMCEILING_SIZE);
    else if (name.equals("Employee"))  SetGif((uint8_t*)Employee, EMPLOYEE_SIZE);
    else if (name.equals("Receptionist"))  SetGif((uint8_t*)Receptionist, RECPTION_SIZE);
    else if (name.equals("RyseAshborne"))  SetGif((uint8_t*)RyseAshborne, RYSEASHBORNE_SIZE);
    else if (name.equals("M'iynt and Kahala"))  SetGif((uint8_t*)M_iyntKahala, MIYNTKAHALA_SIZE);
    
    else SetGif((uint8_t*)Sheep, SHEEPBOSS_SIZE);
}

bool AnimatedSprite::Draw(int x, int y, int disolve) {
    int rc;
    gif_cb ctx = {x, y, disolve};
    if (_gifs[decoder_id].GIFFile.iPos >= _gifs[decoder_id].GIFFile.iSize-1) // no more data exists
    {
        (*_gifs[decoder_id].pfnSeek)(&_gifs[decoder_id].GIFFile, 0); // seek to start
    }
    if (GIFParseInfo(&_gifs[decoder_id], 0))
    {
        _gifs[decoder_id].pUser = &ctx;
        if (_gifs[decoder_id].iError == GIF_EMPTY_FRAME) // don't try to decode it
            return 0;
        rc = DecodeLZW(&_gifs[decoder_id], 0);
        if (rc != 0) // problem
            return -1;
    }
    else
    {
        // The file is "malformed" in that there is a bunch of non-image data after
        // the last frame. Return as if all is well, though if needed getLastError()
        // can be used to see if a frame was actually processed:
        // GIF_SUCCESS -> frame processed, GIF_EMPTY_FRAME -> no frame processed
        if (_gifs[decoder_id].iError == GIF_EMPTY_FRAME)
        {
	        return 0;
        }
        return -1; // error parsing the frame info, we may be at the end of the file
    }
    // Return 1 for more frames or 0 if this was the last frame
    return (_gifs[decoder_id].GIFFile.iPos < _gifs[decoder_id].GIFFile.iSize-10);
}