/**************************************************************************
 * Copyright(c) 1998-2015, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>

#include <TArrayI.h>
#include <TObjArray.h>

#include "AliAODCaloTrigger.h"
#include "AliEMCALGeometry.h"
#include "AliEMCALTriggerConstants.h"
#include "AliEMCALTriggerDataGrid.h"
#include "AliEMCALTriggerPatchInfo.h"
#include "AliEMCALTriggerPatchFinder.h"
#include "AliEMCALTriggerAlgorithm.h"
#include "AliEMCALTriggerRawPatch.h"
#include "AliEmcalTriggerMakerKernel.h"
#include "AliEmcalTriggerSetupInfo.h"
#include "AliLog.h"
#include "AliVCaloCells.h"
#include "AliVCaloTrigger.h"
#include "AliVEvent.h"
#include "AliVVZERO.h"

/// \cond CLASSIMP
ClassImp(AliEmcalTriggerMakerKernel)
/// \endcond

AliEmcalTriggerMakerKernel::AliEmcalTriggerMakerKernel():
  TObject(),
  fBadChannels(),
  fOfflineBadChannels(),
  fFastORPedestal(5000),
  fTriggerBitConfig(NULL),
  fGeometry(NULL),
  fPatchAmplitudes(NULL),
  fPatchADCSimple(NULL),
  fPatchADC(NULL),
  fLevel0TimeMap(NULL),
  fTriggerBitMap(NULL),
  fPatchFinder(NULL),
  fLevel0PatchFinder(NULL),
  fL0MinTime(7),
  fL0MaxTime(10),
  fADCtoGeV(1.),
  fMinCellAmp(0),
  fMinL0FastORAmp(0),
  fMinL1FastORAmp(0),
  fJetPatchsize(16),
  fBkgThreshold(-1),
  fL0Threshold(0),
  fIsMC(kFALSE),
  fDebugLevel(0)
{
  memset(fThresholdConstants, 0, sizeof(Int_t) * 12);
  memset(fL1ThresholdsOffline, 0, sizeof(ULong64_t) * 4);
}

AliEmcalTriggerMakerKernel::~AliEmcalTriggerMakerKernel() {
  delete fPatchAmplitudes;
  delete fPatchADCSimple;
  delete fPatchADC;
  delete fLevel0TimeMap;
  delete fTriggerBitMap;
  delete fPatchFinder;
  delete fLevel0PatchFinder;
  if(fTriggerBitConfig) delete fTriggerBitConfig;
}

void AliEmcalTriggerMakerKernel::Init(){
  fPatchAmplitudes = new AliEMCALTriggerDataGrid<double>;
  fPatchADCSimple = new AliEMCALTriggerDataGrid<double>;
  fPatchADC = new AliEMCALTriggerDataGrid<double>;
  fLevel0TimeMap = new AliEMCALTriggerDataGrid<char>;
  fTriggerBitMap = new AliEMCALTriggerDataGrid<int>;

  // Allocate containers for the ADC values
  int nrows = fGeometry->GetNTotalTRU() * 2;
  std::cout << "Allocating channel grid with 48 columns in eta and " << nrows << " rows in phi" << std::endl;
  std::cout << "Using jet patch size " << fJetPatchsize << std::endl;
  fPatchAmplitudes->Allocate(48, nrows);
  fPatchADC->Allocate(48, nrows);
  fPatchADCSimple->Allocate(48, nrows);
  fLevel0TimeMap->Allocate(48, nrows);
  fTriggerBitMap->Allocate(48, nrows);

  // Initialize patch finder
  fPatchFinder = new AliEMCALTriggerPatchFinder<double>;
  fPatchFinder->AddTriggerAlgorithm(CreateGammaTriggerAlgorithm(0, 63));
  AliEMCALTriggerAlgorithm<double> *jettrigger = CreateJetTriggerAlgorithm(0, 63);
  fPatchFinder->AddTriggerAlgorithm(jettrigger);
  if(fJetPatchsize == 8){
    //jettrigger->SetBitMask(jettrigger->GetBitMask() | 1 << fTriggerBitConfig->GetBkgBit());
    jettrigger->SetBitMask(1 << fTriggerBitConfig->GetJetHighBit() | 1 << fTriggerBitConfig->GetJetLowBit() | 1 << fTriggerBitConfig->GetBkgBit());
  } else {
    fPatchFinder->AddTriggerAlgorithm(CreateBkgTriggerAlgorithm(0, 63));
  }
  if(nrows > 64){
    // Add trigger algorithms for DCAL
    fPatchFinder->AddTriggerAlgorithm(CreateGammaTriggerAlgorithm(64, nrows));
    jettrigger = CreateJetTriggerAlgorithm(64, nrows);
    fPatchFinder->AddTriggerAlgorithm(jettrigger);
    if(fJetPatchsize == 8) {
      //jettrigger->SetBitMask(jettrigger->GetBitMask() | 1 << fTriggerBitConfig->GetBkgBit());
      jettrigger->SetBitMask(1 << fTriggerBitConfig->GetJetHighBit() | 1 << fTriggerBitConfig->GetJetLowBit() | 1 << fTriggerBitConfig->GetBkgBit());
    } else {
      fPatchFinder->AddTriggerAlgorithm(CreateBkgTriggerAlgorithm(64, nrows));
    }
  }

  fLevel0PatchFinder = new AliEMCALTriggerAlgorithm<double>(0, nrows, 0);
  fLevel0PatchFinder->SetPatchSize(2);
  fLevel0PatchFinder->SetSubregionSize(2);
}

void AliEmcalTriggerMakerKernel::ReadOfflineBadChannelFromStream(std::istream& stream)
{
  Short_t absId = 0;

  while (stream.good()) {
    stream >> absId;
    AddOfflineBadChannel(absId);
  }
}

void AliEmcalTriggerMakerKernel::ReadOfflineBadChannelFromFile(const char* fname)
{
  std::ifstream file(fname);
  ReadOfflineBadChannelFromStream(file);
}

void AliEmcalTriggerMakerKernel::ReadFastORBadChannelFromStream(std::istream& stream)
{
  Short_t absId = -1;

  while (stream.good()) {
    stream >> absId;
    AddFastORBadChannel(absId);
  }
}

void AliEmcalTriggerMakerKernel::ReadFastORBadChannelFromFile(const char* fname)
{
  std::ifstream file(fname);
  ReadFastORBadChannelFromStream(file);
}

void AliEmcalTriggerMakerKernel::SetFastORPedestal(Short_t absId, Float_t ped)
{
  if (absId < 0 || absId >= fFastORPedestal.GetSize()) {
    AliWarning(Form("Abs. ID %d out of range (0,5000)", absId));
    return;
  }
  fFastORPedestal[absId] = ped;
}

void AliEmcalTriggerMakerKernel::ReadFastORPedestalFromStream(std::istream& stream)
{
  Short_t absId = 0;
  Float_t ped = 0;
  while (stream.good()) {
    stream >> ped;
    SetFastORPedestal(absId, ped);
    absId++;
  }
}

void AliEmcalTriggerMakerKernel::ReadFastORPedestalFromFile(const char* fname)
{
  std::ifstream file(fname);
  ReadFastORPedestalFromStream(file);
}

void AliEmcalTriggerMakerKernel::Reset(){
  fPatchAmplitudes->Reset();
  fPatchADC->Reset();
  fPatchADCSimple->Reset();
  fLevel0TimeMap->Reset();
  fTriggerBitMap->Reset();
  memset(fL1ThresholdsOffline, 0, sizeof(ULong64_t) * 4);
}

void AliEmcalTriggerMakerKernel::ReadTriggerData(AliVCaloTrigger *trigger){
  trigger->Reset();
  Int_t globCol=-1, globRow=-1;
  Int_t adcAmp=-1, bitmap = 0;
  while(trigger->Next()){
    // get position in global 2x2 tower coordinates
    // A0 left bottom (0,0)
    trigger->GetPosition(globCol, globRow);
    Int_t absId = -1;
    fGeometry->GetAbsFastORIndexFromPositionInEMCAL(globCol, globRow, absId);
    // exclude channel completely if it is masked as hot channel
    if (fBadChannels.find(absId) != fBadChannels.end()) continue;
    // for some strange reason some ADC amps are initialized in reconstruction
    // as -1, neglect those
    trigger->GetL1TimeSum(adcAmp);
    if (adcAmp < 0) adcAmp = 0;

    if (adcAmp >= fMinL1FastORAmp) {
      (*fPatchADC)(globCol,globRow) = adcAmp;
      trigger->GetTriggerBits(bitmap);
      (*fTriggerBitMap)(globCol, globRow) = bitmap;
    }

    // Handling for L0 triggers
    // For the ADC value we use fCaloTriggers->GetAmplitude()
    // In data, all patches which have 4 TRUs with proper level0 times are
    // valid trigger patches. Therefore we need to check all neighbors for
    // the level0 times, not only the bottom left. In order to obtain this
    // information, a lookup table with the L0 times for each TRU is created
    Float_t amplitude(0);
    trigger->GetAmplitude(amplitude);
    amplitude *= 4; // values are shifted by 2 bits to fit in a 10 bit word (on the hardware side)
    amplitude -= fFastORPedestal[absId];
    if(amplitude < 0) amplitude = 0;
    if (amplitude >= fMinL0FastORAmp) {
      (*fPatchAmplitudes)(globCol,globRow) = amplitude;
      Int_t nl0times(0);
      trigger->GetNL0Times(nl0times);
      if(nl0times){
        TArrayI l0times(nl0times);
        trigger->GetL0Times(l0times.GetArray());
        for(int itime = 0; itime < nl0times; itime++){
          (*fLevel0TimeMap)(globCol,globRow) = static_cast<Char_t>(l0times[itime]);
          break;
        }
      }
    }
  }
}

void AliEmcalTriggerMakerKernel::ReadCellData(AliVCaloCells *cells){
  // fill the patch ADCs from cells
  Int_t nCell = cells->GetNumberOfCells();
  for(Int_t iCell = 0; iCell < nCell; ++iCell) {
    // get the cell info, based in index in array
    Short_t cellId = cells->GetCellNumber(iCell);

    // Check bad channel map
    if (fOfflineBadChannels.find(cellId) != fOfflineBadChannels.end()) {
      AliDebug(10, Form("%hd is a bad channel, skipped.", cellId));
      continue;
    }

    Double_t amp = cells->GetAmplitude(iCell);
    // get position
    Int_t absId=-1;
    fGeometry->GetFastORIndexFromCellIndex(cellId, absId);
    Int_t globCol=-1, globRow=-1;
    fGeometry->GetPositionInEMCALFromAbsFastORIndex(absId, globCol, globRow);
    // add
    amp /= fADCtoGeV;
    if (amp >= fMinCellAmp) (*fPatchADCSimple)(globCol,globRow) += amp;
  }
}

void AliEmcalTriggerMakerKernel::BuildL1ThresholdsOffline(const AliVVZERO *vzerodata){
  // get the V0 value and compute and set the offline thresholds
  // get V0, compute thresholds and save them as global parameters
  ULong64_t v0S = vzerodata->GetTriggerChargeA() + vzerodata->GetTriggerChargeC();
  for (Int_t i = 0; i < 4; ++i) {
    // A*V0^2/2^32+B*V0/2^16+C
    fL1ThresholdsOffline[i]= ( ((ULong64_t)fThresholdConstants[i][0]) * v0S * v0S ) >> 32;
    fL1ThresholdsOffline[i] += ( ((ULong64_t)fThresholdConstants[i][1]) * v0S ) >> 16;
    fL1ThresholdsOffline[i] += ((ULong64_t)fThresholdConstants[i][2]);
  }
}

TObjArray *AliEmcalTriggerMakerKernel::CreateTriggerPatches(const AliVEvent *inputevent, Bool_t useL0amp){
  //std::cout << "Finding trigger patches" << std::endl;
  //AliEMCALTriggerPatchInfo *trigger, *triggerMainJet, *triggerMainGamma, *triggerMainLevel0;
  //AliEMCALTriggerPatchInfo *triggerMainJetSimple, *triggerMainGammaSimple;

  if (useL0amp) {
    fADCtoGeV = EMCALTrigger::kEMCL0ADCtoGeV_AP;
  }
  else {
    fADCtoGeV = EMCALTrigger::kEMCL1ADCtoGeV;
  }

  Double_t vertexpos[3];
  inputevent->GetPrimaryVertex()->GetXYZ(vertexpos);
  TVector3 vertexvec(vertexpos);

  Int_t isMC = fIsMC ? 1 : 0;
  Int_t offset = (1 - isMC) * fTriggerBitConfig->GetTriggerTypesEnd();

  // Create trigger bit masks. They are needed later to remove
  // trigger bits from the trigger bit mask for non-matching patch types
  Int_t jetPatchMask =  1 << fTriggerBitConfig->GetJetHighBit()
      | 1 << fTriggerBitConfig->GetJetLowBit()
      | 1 << (fTriggerBitConfig->GetJetHighBit() + fTriggerBitConfig->GetTriggerTypesEnd())
      | 1 << (fTriggerBitConfig->GetJetLowBit() + fTriggerBitConfig->GetTriggerTypesEnd()),
      gammaPatchMask = 1 << fTriggerBitConfig->GetGammaHighBit()
      | 1 << fTriggerBitConfig->GetGammaLowBit()
      | 1 << (fTriggerBitConfig->GetGammaHighBit() + fTriggerBitConfig->GetTriggerTypesEnd())
      | 1 << (fTriggerBitConfig->GetGammaLowBit() + fTriggerBitConfig->GetTriggerTypesEnd()),
      bkgPatchMask = 1 << fTriggerBitConfig->GetBkgBit(),
      l0PatchMask = 1 << fTriggerBitConfig->GetLevel0Bit();

  std::vector<AliEMCALTriggerRawPatch> patches;
  if (useL0amp) {
    patches = fPatchFinder->FindPatches(*fPatchAmplitudes, *fPatchADCSimple);
  }
  else {
    patches = fPatchFinder->FindPatches(*fPatchADC, *fPatchADCSimple);
  }
  TObjArray *result = new TObjArray(1000);
  result->SetOwner(kTRUE);
  for(std::vector<AliEMCALTriggerRawPatch>::iterator patchit = patches.begin(); patchit != patches.end(); ++patchit){
    // Apply offline and recalc selection
    // Remove unwanted bits from the online bits (gamma bits from jet patches and vice versa)
    Int_t offlinebits = 0, onlinebits = (*fTriggerBitMap)(patchit->GetColStart(), patchit->GetRowStart());
    if(IsGammaPatch(*patchit)){
      if(patchit->GetADC() > fL1ThresholdsOffline[1]) SETBIT(offlinebits, AliEMCALTriggerPatchInfo::kRecalcOffset + fTriggerBitConfig->GetGammaHighBit());
      if(patchit->GetOfflineADC() > fL1ThresholdsOffline[1]) SETBIT(offlinebits, AliEMCALTriggerPatchInfo::kOfflineOffset + fTriggerBitConfig->GetGammaHighBit());
      if(patchit->GetADC() > fL1ThresholdsOffline[3]) SETBIT(offlinebits, AliEMCALTriggerPatchInfo::kRecalcOffset + fTriggerBitConfig->GetGammaLowBit());
      if(patchit->GetOfflineADC() > fL1ThresholdsOffline[3]) SETBIT(offlinebits, AliEMCALTriggerPatchInfo::kOfflineOffset + fTriggerBitConfig->GetGammaLowBit());
      onlinebits &= gammaPatchMask;
    }
    if (IsJetPatch(*patchit)){
      if(patchit->GetADC() > fL1ThresholdsOffline[0]) SETBIT(offlinebits, AliEMCALTriggerPatchInfo::kRecalcOffset + fTriggerBitConfig->GetJetHighBit());
      if(patchit->GetOfflineADC() > fL1ThresholdsOffline[0]) SETBIT(offlinebits, AliEMCALTriggerPatchInfo::kOfflineOffset + fTriggerBitConfig->GetJetHighBit());
      if(patchit->GetADC() > fL1ThresholdsOffline[2]) SETBIT(offlinebits, AliEMCALTriggerPatchInfo::kRecalcOffset + fTriggerBitConfig->GetJetLowBit());
      if(patchit->GetOfflineADC() > fL1ThresholdsOffline[2]) SETBIT(offlinebits, AliEMCALTriggerPatchInfo::kOfflineOffset + fTriggerBitConfig->GetJetLowBit());
      onlinebits &= jetPatchMask;
    }
    if (IsBkgPatch(*patchit)){
      if(patchit->GetADC() > fBkgThreshold) SETBIT(offlinebits, AliEMCALTriggerPatchInfo::kRecalcOffset + fTriggerBitConfig->GetBkgBit());
      if(patchit->GetOfflineADC() > fBkgThreshold) SETBIT(offlinebits, AliEMCALTriggerPatchInfo::kOfflineOffset + fTriggerBitConfig->GetBkgBit());
      onlinebits &= bkgPatchMask;
    }
    // convert
    AliEMCALTriggerPatchInfo *fullpatch = AliEMCALTriggerPatchInfo::CreateAndInitialize(patchit->GetColStart(), patchit->GetRowStart(),
        patchit->GetPatchSize(), patchit->GetADC(), patchit->GetOfflineADC(), patchit->GetOfflineADC() * fADCtoGeV,
        onlinebits | offlinebits, vertexvec, fGeometry);
    fullpatch->SetTriggerBitConfig(fTriggerBitConfig);
    fullpatch->SetOffSet(offset);
    result->Add(fullpatch);
  }

  // Find Level0 patches
  std::vector<AliEMCALTriggerRawPatch> l0patches = fLevel0PatchFinder->FindPatches(*fPatchAmplitudes, *fPatchADCSimple);
  for(std::vector<AliEMCALTriggerRawPatch>::iterator patchit = l0patches.begin(); patchit != l0patches.end(); ++patchit){
    Int_t offlinebits = 0, onlinebits = 0;
    ELevel0TriggerStatus_t L0status = CheckForL0(patchit->GetColStart(), patchit->GetRowStart());
    if (L0status == kNotLevel0) continue;
    if (L0status == kLevel0Fired) SETBIT(onlinebits, fTriggerBitConfig->GetLevel0Bit());
    if (patchit->GetADC() > fL0Threshold) SETBIT(offlinebits, AliEMCALTriggerPatchInfo::kRecalcOffset + fTriggerBitConfig->GetLevel0Bit());
    if (patchit->GetOfflineADC() > fL0Threshold) SETBIT(offlinebits, AliEMCALTriggerPatchInfo::kOfflineOffset + fTriggerBitConfig->GetLevel0Bit());

    AliEMCALTriggerPatchInfo *fullpatch = AliEMCALTriggerPatchInfo::CreateAndInitialize(patchit->GetColStart(), patchit->GetRowStart(),
        patchit->GetPatchSize(), patchit->GetADC(), patchit->GetOfflineADC(), patchit->GetOfflineADC() * fADCtoGeV,
        onlinebits | offlinebits, vertexvec, fGeometry);
    fullpatch->SetTriggerBitConfig(fTriggerBitConfig);
    result->Add(fullpatch);
  }
  // std::cout << "Finished finding trigger patches" << std::endl;
  return result;
}

AliEmcalTriggerMakerKernel::ELevel0TriggerStatus_t AliEmcalTriggerMakerKernel::CheckForL0(Int_t col, Int_t row) const {
  ELevel0TriggerStatus_t result = kLevel0Candidate;

  if(col < 0 || row < 0){
    AliError(Form("Patch outside range [col %d, row %d]", col, row));
    return kNotLevel0;
  }
  Int_t truref(-1), trumod(-1), absFastor(-1), adc(-1);
  fGeometry->GetAbsFastORIndexFromPositionInEMCAL(col, row, absFastor);
  fGeometry->GetTRUFromAbsFastORIndex(absFastor, truref, adc);
  int nvalid(0);
  const int kNRowsPhi = fGeometry->GetNTotalTRU() * 2;
  for(int ipos = 0; ipos < 2; ipos++){
    if(row + ipos >= kNRowsPhi) continue;    // boundary check
    for(int jpos = 0; jpos < 2; jpos++){
      if(col + jpos >= kColsEta) continue;  // boundary check
      // Check whether we are in the same TRU
      trumod = -1;
      fGeometry->GetAbsFastORIndexFromPositionInEMCAL(col+jpos, row+ipos, absFastor);
      fGeometry->GetTRUFromAbsFastORIndex(absFastor, trumod, adc);
      if(trumod != truref) {
        result = kNotLevel0;
        return result;
      }
      if(col + jpos >= kColsEta) AliError(Form("Boundary error in col [%d, %d + %d]", col + jpos, col, jpos));
      if(row + ipos >= kNRowsPhi) AliError(Form("Boundary error in row [%d, %d + %d]", row + ipos, row, ipos));
      Char_t l0times = (*fLevel0TimeMap)(col + jpos,row + ipos);
      if(l0times > fL0MinTime && l0times < fL0MaxTime) nvalid++;
    }
  }
  if (nvalid == 4) result = kLevel0Fired;
  return result;
}

AliEMCALTriggerAlgorithm<double> *AliEmcalTriggerMakerKernel::CreateGammaTriggerAlgorithm(int rowmin, int rowmax) const {
  AliEMCALTriggerAlgorithm<double> *result = new AliEMCALTriggerAlgorithm<double>(rowmin, rowmax, 0);
  result->SetPatchSize(2);
  result->SetSubregionSize(1);
  result->SetBitMask(1<<fTriggerBitConfig->GetGammaHighBit() | 1<<fTriggerBitConfig->GetGammaLowBit());
  return result;
}

AliEMCALTriggerAlgorithm<double> *AliEmcalTriggerMakerKernel::CreateJetTriggerAlgorithm(int rowmin, int rowmax) const {
  AliEMCALTriggerAlgorithm<double> *result = new AliEMCALTriggerAlgorithm<double>(rowmin, rowmax, 0);
  result->SetPatchSize(fJetPatchsize);
  result->SetSubregionSize(4);
  result->SetBitMask(1<<fTriggerBitConfig->GetJetHighBit() | 1<<fTriggerBitConfig->GetJetLowBit());
  return result;
}

AliEMCALTriggerAlgorithm<double> *AliEmcalTriggerMakerKernel::CreateBkgTriggerAlgorithm(int rowmin, int rowmax) const {
  AliEMCALTriggerAlgorithm<double> *result = new AliEMCALTriggerAlgorithm<double>(rowmin, rowmax, 0);
  result->SetPatchSize(8);
  result->SetSubregionSize(4);
  result->SetBitMask(1<<fTriggerBitConfig->GetBkgBit());
  return result;
}

Bool_t AliEmcalTriggerMakerKernel::IsGammaPatch(const AliEMCALTriggerRawPatch &patch) const {
  ULong_t bitmask = patch.GetBitmask(), testmask = 1 << fTriggerBitConfig->GetGammaHighBit() | 1 << fTriggerBitConfig->GetGammaLowBit();
  return bitmask & testmask;
}

Bool_t AliEmcalTriggerMakerKernel::IsJetPatch(const AliEMCALTriggerRawPatch &patch) const {
  ULong_t bitmask = patch.GetBitmask(), testmask = 1 << fTriggerBitConfig->GetJetHighBit() | 1 << fTriggerBitConfig->GetJetLowBit();
  return bitmask & testmask;
}

Bool_t AliEmcalTriggerMakerKernel::IsBkgPatch(const AliEMCALTriggerRawPatch &patch) const {
  ULong_t bitmask = patch.GetBitmask(), testmask = 1 << fTriggerBitConfig->GetBkgBit();
  return bitmask & testmask;
}