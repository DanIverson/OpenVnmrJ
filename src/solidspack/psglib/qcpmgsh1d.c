/*qcpmg1d.c - A sequence to do direct polarization and an echo with
              CPMG detection and TPPM or SPINAL decoupling.

          Larsen et al, J. Phys Chem A Vol 101, 46,8597-8606, 1997.

              Phase cycle provided by A. Lipton
              D.Rice 11/15/06                                           

              Added shaped refocusing pulses for Shaped (WURST) CPMG        
              D. Rice 05/05/11                                      */

#include "standard.h"
#include "solidstandard.h"
#include "pboxpulse.h"

// Define Values for Phasetables

static int table1[4] = {0,0,0,0};                           // phHdec
static int table2[4] = {2,3,0,1};                           // phX90
static int table3[8] = {1,0,3,2,3,2,0,1};                   // phXecho
static int table4[16] = {1,0,3,2,3,2,1,0,3,2,1,0,1,0,3,2};  // phXcpmg
static int table5[4] = {0,1,2,3};                           // phRec

#define phHdec t1
#define phX90 t2
#define phXecho t3
#define phXcpmg t4
#define phRec t5

void pulsesequence() {

// Set the Maximum Dynamic Table Number

   settablenumber(10);
   setvvarnumber(30);

// Define Variables and Objects and Get Parameter Values

   WMPA mp = getcpmg("cpmgX");
   strcpy(mp.ch,"obs"); 
   putCmd("chXcpmg='obs'\n");
   int chnl = 1;                      

   double aXecho = getval("aXecho");
   double t1Xechoinit = getval("t1Xecho");
   double pwXecho = getval("pwXecho");
   double t2Xechoinit = getval("t2Xecho");
   double t1Xecho  = t1Xechoinit - pwXecho/2.0 - getval("pwX90")/2.0;
   if (t1Xecho < 0.0) t1Xecho = 0.0;
   double t2Xecho  = t2Xechoinit - pwXecho/2.0 - mp.r1 - mp.t2 - getval("ad");
   if (t2Xecho < 0.0) t2Xecho = 0.0;

   PBOXPULSE shp1 = getpboxpulse("shp1X",0,0);
   shp1.db = tpwr;
   shp1.a = mp.a;
   shp1.pw = mp.pw;
   shp1 = update_PBOXPULSE(shp1,0.1);
   shp1.t1 = 0.0;
   shp1.t2 = 0.0;
   putCmd("aXshp1 = aXcpmg");
   putCmd("pwXshp1 = pwXcpmg");
   putCmd("dbXshp1 = tpwr");

   DSEQ dec = getdseq("H");
   strcpy(dec.t.ch,"dec");
   putCmd("chHtppm='dec'\n"); 
   strcpy(dec.s.ch,"dec");
   putCmd("chHspinal='dec'\n");

//--------------------------------------
// Copy Current Parameters to Processed
//-------------------------------------

   putCmd("groupcopy('current','processed','acquisition')");

// Dutycycle Protection

   DUTY d = init_dutycycle();
   d.dutyon = getval("pwX90") + pwXecho + (mp.cycles - 1)*shp1.pw; 
   d.dutyoff = d1 + 4.0e-6;
   d.c1 = d.c1 + (!strcmp(dec.seq,"tppm"));
   d.c1 = d.c1 + ((!strcmp(dec.seq,"tppm")) && (dec.t.a > 0.0));
   d.t1 = t1Xecho + t2Xecho + getval("rd") + getval("ad") + 
          at - (mp.cycles - 1)*shp1.pw;
   d.c2 = d.c2 + (!strcmp(dec.seq,"spinal"));
   d.c2 = d.c2 + ((!strcmp(dec.seq,"spinal")) && (dec.s.a > 0.0));
   d.t2 = t1Xecho + t2Xecho + getval("rd") + getval("ad") + 
          at - (mp.cycles - 1)*shp1.pw;
   d = update_dutycycle(d);
   abort_dutycycle(d,10.0);

// Set Phase Tables

   settable(phHdec,4,table1);
   settable(phX90,4,table2);
   settable(phXecho,8,table3);
   settable(phXcpmg,16,table4);
   settable(phRec,4,table5);
   setreceiver(phRec);

   initval(0.0,v1);

// Begin Sequence

   txphase(phX90); decphase(phHdec);
   obspwrf(getval("aX90"));
   obsunblank(); decunblank(); _unblank34();
   delay(d1);
   sp1on(); delay(2.0e-6); sp1off(); delay(2.0e-6);

// H Decoupler on Before the Echo

   _dseqon(dec);

// X Direct Polarization

   rcvroff();
   delay(rof1);
   rgpulse(getval("pwX90"),phX90,0.0,0.0);

// X Hahn Echo

   txphase(phXecho);
   obspwrf(aXecho);
   delay(t1Xecho);
   rgpulse(pwXecho,phXecho,0.0,0.0);
   delay(t2Xecho);
   txphase(phXcpmg);

// Apply CPMG Cycles

   obsblank(); _blank34();
   delay(mp.r1);
   startacq(getval("ad"));
   getelem(phXcpmg,ct,mp.vphase);
   txphase(mp.vphase);
   gen_RFpwrf(mp.a,chnl);
   acquire(mp.npa1, 2.0e-7);
   delay(mp.t1);
   loop(mp.vcycles,mp.vcount);
      rcvroff();
      gen_RFunblank(chnl);
      gen_RFphase(mp.vphase, chnl);
      delay(mp.r1);
      shapedpulse(shp1.pattern,shp1.pw,phXcpmg,0.0,0.0); 
      delay(mp.r2);
      rcvron();
      delay(mp.dtaua);
      acquire(mp.npa, 2.0e-7);
   endloop(mp.vcount);
   endacq();
   _dseqoff(dec);
   obsunblank(); decunblank(); _unblank34();
}

