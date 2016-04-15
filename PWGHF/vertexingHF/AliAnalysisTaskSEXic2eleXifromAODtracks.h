#ifndef ALIANALYSISTASKSEXIC2ELEXIFROMAODTRACKS_H
#define ALIANALYSISTASKSEXIC2ELEXIFROMAODTRACKS_H

/**************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
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

/* $Id$ */ 

#include "TROOT.h"
#include "TVector.h"
#include "TSystem.h"

#include "AliAnalysisTaskSE.h"
#include "AliAODEvent.h"
#include "AliPID.h"
#include "AliRDHFCutsXictoeleXifromAODtracks.h"

/// \class AliAnalysisTaskSEXic2eleXifromAODtracks

class THnSparse;
class TH1F;
class TClonesArray;
class AliAODRecoCascadeHF;
class AliESDVertex;
class AliAODMCParticle;
class AliEventPoolManager;
class AliNormalizationCounter;
class TLorentzVector;

class AliAnalysisTaskSEXic2eleXifromAODtracks : public AliAnalysisTaskSE 
{
 public:
  AliAnalysisTaskSEXic2eleXifromAODtracks();
  AliAnalysisTaskSEXic2eleXifromAODtracks(const Char_t* name, AliRDHFCutsXictoeleXifromAODtracks* cuts, Bool_t writeVariableTree=kTRUE);
  virtual ~AliAnalysisTaskSEXic2eleXifromAODtracks();

  /// Implementation of interface methods
  virtual void UserCreateOutputObjects();
  virtual void Init();
  virtual void LocalInit() {Init();}
  virtual void UserExec(Option_t *option);
  virtual void Terminate(Option_t *option);

  void FillROOTObjects(AliAODRecoCascadeHF *elobj, AliAODcascade *casc, AliAODTrack *trk, AliAODTrack *trkpid, AliAODEvent *event,  TClonesArray *mcArray);
  void FillMixROOTObjects(TLorentzVector *et, TLorentzVector *ev, TVector *tinfo, TVector *vinfo, Int_t charge);
  void FillElectronROOTObjects(AliAODTrack *trk, AliAODTrack *trkpid, AliAODEvent *event, TClonesArray *mcArray);
  void FillCascROOTObjects(AliAODcascade *casc, TClonesArray *mcArray);
  void FillMCROOTObjects(AliAODMCParticle *part, AliAODMCParticle *mcepart, AliAODMCParticle *mcv0part, Int_t decaytype);
  void FillMCEleROOTObjects(AliAODMCParticle *mcepart, TClonesArray *mcArray);
  void FillMCCascROOTObjects(AliAODMCParticle *mccpart, TClonesArray *mcArray);
  void FillMCGenPairROOTObjects(AliAODMCParticle *mcparte, AliAODMCParticle* mcpartv, TClonesArray *mcArray);
  Bool_t MakeMCAnalysis(TClonesArray *mcArray);
  void MakeAnalysis(AliAODEvent *aod, TClonesArray *mcArray);

  void SelectCascade( const AliVEvent *event,Int_t nCasc,Int_t &nSeleCasc, Bool_t *seleCascFlags, TClonesArray *mcArray);
  void SelectTrack( const AliVEvent *event, Int_t trkEntries, Int_t &nSeleTrks,Bool_t *seleFlags, TClonesArray *mcArray);

  /// set MC usage
  void SetMC(Bool_t theMCon) {fUseMCInfo = theMCon;}
  Bool_t GetMC() const {return fUseMCInfo;}
  void SetUseCentralityV0M(Bool_t centon) {fUseCentralityV0M = centon;}
  Bool_t GetUseCentralityV0M() const {return fUseCentralityV0M;}
  void SetWriteEachVariableTree(Bool_t a) {fWriteEachVariableTree = a;}
  Bool_t GetWriteEachVariableTree() const {return fWriteEachVariableTree;}
  void SetWriteMCVariableTree(Bool_t a) {fWriteMCVariableTree = a;}
  Bool_t GetWriteMCVariableTree() const {return fWriteMCVariableTree;}
  void SetMCEventType(Int_t theevt) {fMCEventType = theevt;}
  Int_t GetMCEventType() const {return fMCEventType;}
  void SetMCDoPairAnalysis(Bool_t a) {fMCDoPairAnalysis = a;}
  Bool_t GetMCDoPairAnalysis() const {return fMCDoPairAnalysis;}

  void SetReconstructPrimVert(Bool_t a) { fReconstructPrimVert=a; }

  AliAODRecoCascadeHF* MakeCascadeHF(AliAODcascade *casc, AliAODTrack *trk, AliAODTrack *trkpid, AliAODEvent *aod, AliAODVertex *vert);
  AliAODVertex* ReconstructSecondaryVertex(AliAODcascade *casc, AliAODTrack *trk, AliAODEvent *aod);
	Int_t MatchToMC(AliAODRecoCascadeHF *elobj, TClonesArray *mcArray, Int_t *pdgarray_ele, Int_t *pdgarray_casc, Int_t *labelarray_ele, Int_t *labelarray_casc,  Int_t &ngen_ele, Int_t &ngen_casc);
	Int_t MatchToMCCascade(AliAODcascade *theCascade, Int_t pdgabscasc, Int_t *pdgDgcasc, Int_t *pdgDgv0, TClonesArray *mcArray) const;
	void	GetMCDecayHistory(AliAODMCParticle *mcpart, TClonesArray *mcArray, Int_t *pdgarray, Int_t *labelarray, Int_t &ngen);

  void StoreGlobalTrackReference(AliAODTrack *track, Int_t id);
  void ResetGlobalTrackReference();

  /// mixing
  void SetEventMixingWithPools(){fDoEventMixing=1;}
  void SetEventMixingOff(){fDoEventMixing=0;}
  void SetMixWithoutConversionFlag(Bool_t a){fMixWithoutConversionFlag=a;}
	void SetNumberOfEventsForMixing(Int_t events){fNumberOfEventsForMixing=events;}
	void SetPoolPVzBinLimits(Int_t Nzvtxbins,const Double_t *ZvtxBins){
		fNzVtxBins = Nzvtxbins;
		for(int ix = 0;ix<fNzVtxBins+1;ix++){fZvtxBins[ix] = ZvtxBins[ix];}
	}
	void SetPoolCentBinLimits(Int_t Ncentbins,const Double_t *CentBins){
		fNCentBins = Ncentbins;
		for(int ix = 0;ix<fNCentBins+1;ix++){fCentBins[ix] = CentBins[ix];}
	}
  void DoEventMixingWithPools(Int_t index);
  void ResetPool(Int_t poolIndex);
  Int_t GetPoolIndex(Double_t zvert, Double_t mult);


 private:

  AliAnalysisTaskSEXic2eleXifromAODtracks(const AliAnalysisTaskSEXic2eleXifromAODtracks &source);
  AliAnalysisTaskSEXic2eleXifromAODtracks& operator=(const AliAnalysisTaskSEXic2eleXifromAODtracks& source); 

  void DefineTreeVariables();
  void DefineEleTreeVariables();
  void DefineCascTreeVariables();
  void DefineMCTreeVariables();
  void DefineMCEleTreeVariables();
  void DefineMCCascTreeVariables();
  void DefineMCGenPairTreeVariables();
  void DefineCorrelationTreeVariables();
  void DefineGeneralHistograms();
  void DefineAnalysisHistograms();
  Bool_t HaveCharmInHistory(Int_t *history);
  Bool_t HaveBottomInHistory(Int_t *history);
  Int_t FromSemileptonicDecays(Int_t *history);

  AliAODVertex *CallPrimaryVertex(AliAODcascade *casc, AliAODTrack *trk, AliAODEvent *evt);
  AliAODVertex* PrimaryVertex(const TObjArray *trkArray,AliVEvent *event);

  Bool_t fUseMCInfo;                 /// Use MC info
  TList *fOutput;                    //!<! User output slot 1 // general histos
  TList *fOutputAll;                 //!<! User Output slot 3  //analysis histograms 
  TList *fListCuts;                  //!<! User output slot 2 // Cuts 
  TH1F *fCEvents;                    //!<! Histogram to check selected events
  TH1F *fHTrigger;                   //!<! Histogram to check Trigger
  TH1F *fHCentrality;                //!<! Histogram to check Centrality
  AliRDHFCutsXictoeleXifromAODtracks *fAnalCuts;// Cuts - sent to output slot 2
  Bool_t fIsEventSelected;          /// flag for event selected
  Bool_t    fWriteVariableTree;     /// flag to decide whether to write the candidate variables on a tree variables
  TTree    *fVariablesTree;         //!<! tree of the candidate variables after track selection on output slot 4
  Bool_t    fWriteEachVariableTree;     /// flag to decide whether to write the candidate variables on a tree variables
  Bool_t    fWriteMCVariableTree;     /// flag to decide whether to write the candidate variables on a tree variables
  TTree    *fEleVariablesTree;         //!<! tree of the candidate variables after track selection on output slot 4
  TTree    *fCascVariablesTree;         //!<! tree of the candidate variables after track selection on output slot 4
  TTree    *fMCVariablesTree;         //!<! tree of the candidate variables after track selection on output slot 4
  TTree    *fMCEleVariablesTree;         //!<! tree of the candidate variables after track selection on output slot 4
  TTree    *fMCCascVariablesTree;         //!<! tree of the candidate variables after track selection on output slot 4
  TTree    *fMCGenPairVariablesTree;         //!<! tree of mcArray analysis pair tree
  TTree* fCorrelationVariablesTree;         //!<! Correlation variable tree under histo object list
  Bool_t fReconstructPrimVert;       /// Reconstruct primary vertex excluding candidate tracks
  Bool_t fIsMB;       /// MB trigger event
  Bool_t fIsSemi;     /// SemiCentral trigger event
  Bool_t fIsCent;     /// Central trigger event
  Bool_t fIsINT7;     /// INT7 trigger event
  Bool_t fIsEMC7;     /// EMC7 trigger event
  Float_t *fCandidateVariables;   //!<! variables to be written to the tree
  Float_t *fCandidateEleVariables;   //!<! variables to be written to the tree
  Float_t *fCandidateCascVariables;   //!<! variables to be written to the tree
  Float_t *fCandidateMCVariables;   //!<! variables to be written to the tree
  Float_t *fCandidateMCEleVariables;   //!<! variables to be written to the tree
  Float_t *fCandidateMCCascVariables;   //!<! variables to be written to the tree
  Float_t *fCandidateMCGenPairVariables;   //!<! variables to be written to the tree
  Float_t *fCorrelationVariables;   //!<! Correlation variables to be written to the tree
  AliAODVertex *fVtx1;            /// primary vertex
  AliESDVertex *fV1;              /// primary vertex
  Float_t  fVtxZ;         /// zVertex
  Double_t fBzkG;                 /// magnetic field value [kG]
  Float_t  fCentrality;           /// Centrality
  Int_t  fRunNumber;           /// Run Number
  Float_t  fTriggerCheck;         /// Stores trigger information
  Bool_t  fUseCentralityV0M;         /// Stores trigger information
  Int_t  fEvNumberCounter;         /// EvNumber counter
	Int_t fMCEventType; ///MC eventtype to analyze 1: ccbar 2: bbbar 
	Bool_t fMCDoPairAnalysis; /// Flag to do pair analysis

  //--------------------- My histograms ------------------
  THnSparse* fHistoEleXiMass;         //!<! e-Xi mass spectra
  THnSparse* fHistoEleXiMassRS;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWS;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRSMix;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSMix;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRSSide;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSSide;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRS1;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWS1;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRSMix1;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSMix1;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRSSide1;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSSide1;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRS2;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWS2;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRSMix2;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSMix2;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRSSide2;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSSide2;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassAway;         //!<! e-Xi mass spectra
  THnSparse* fHistoEleXiMassRSAway;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSAway;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRSMixAway;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSMixAway;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRSSideAway;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSSideAway;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRS1Away;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWS1Away;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRSMix1Away;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSMix1Away;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRSSide1Away;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSSide1Away;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRS2Away;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWS2Away;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRSMix2Away;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSMix2Away;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassRSSide2Away;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassWSSide2Away;         //!<! e-Xi mass spectra (wrong-sign)

  THnSparse* fHistoEleXiMassvsElePtRS;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassvsElePtWS;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassvsElePtRSMix;         //!<! e-Xi mass-ept spectra (right-sign)
  THnSparse* fHistoEleXiMassvsElePtWSMix;         //!<! e-Xi mass-ept spectra (wrong-sign)
  THnSparse* fHistoEleXiMassvsElePtRSSide;         //!<! e-Xi mass-ept spectra (right-sign)
  THnSparse* fHistoEleXiMassvsElePtWSSide;         //!<! e-Xi mass-ept spectra (wrong-sign)
  THnSparse* fHistoEleXiMassvsElePtRS1;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassvsElePtWS1;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassvsElePtRSMix1;         //!<! e-Xi mass-ept spectra (right-sign)
  THnSparse* fHistoEleXiMassvsElePtWSMix1;         //!<! e-Xi mass-ept spectra (wrong-sign)
  THnSparse* fHistoEleXiMassvsElePtRSSide1;         //!<! e-Xi mass-ept spectra (right-sign)
  THnSparse* fHistoEleXiMassvsElePtWSSide1;         //!<! e-Xi mass-ept spectra (wrong-sign)
  THnSparse* fHistoEleXiMassvsElePtRS2;         //!<! e-Xi mass spectra (right-sign)
  THnSparse* fHistoEleXiMassvsElePtWS2;         //!<! e-Xi mass spectra (wrong-sign)
  THnSparse* fHistoEleXiMassvsElePtRSMix2;         //!<! e-Xi mass-ept spectra (right-sign)
  THnSparse* fHistoEleXiMassvsElePtWSMix2;         //!<! e-Xi mass-ept spectra (wrong-sign)
  THnSparse* fHistoEleXiMassvsElePtRSSide2;         //!<! e-Xi mass-ept spectra (right-sign)
  THnSparse* fHistoEleXiMassvsElePtWSSide2;         //!<! e-Xi mass-ept spectra (wrong-sign)
  TH2F* fHistoElePtRS;         //!<! e spectra (right-sign)
  TH2F* fHistoElePtWS;         //!<! e spectra (wrong-sign)
  TH2F* fHistoElePtRSMix;         //!<! e spectra (right-sign, mix)
  TH2F* fHistoElePtWSMix;         //!<! e spectra (wrong-sign, mix)
  THnSparse* fHistoEleXiMassMCS;         //!<! e-Xi mass spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassMCS1;         //!<! e-Xi mass spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassMCS2;         //!<! e-Xi mass spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassXibMCS;         //!<! e-Xi mass spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassXibMCS1;         //!<! e-Xi mass spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassXibMCS2;         //!<! e-Xi mass spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassPromptMCS;         //!<! e-Xi mass spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassPromptMCS1;         //!<! e-Xi mass spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassPromptMCS2;         //!<! e-Xi mass spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassBFeeddownMCS;         //!<! e-Xi mass spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassBFeeddownMCS1;         //!<! e-Xi mass spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassBFeeddownMCS2;         //!<! e-Xi mass spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassMCGen;         //!<! e-Xi mass spectra (Efficiency denominator)
  THnSparse* fHistoEleXiMassvsElePtMCS;         //!<! e-Xi mass-ept spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassvsElePtMCGen;         //!<! e-Xi mass-ept spectra (Efficiency denominator)
  THnSparse* fHistoEleXiMassvsElePtMCS1;         //!<! e-Xi mass-ept spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassvsElePtMCGen1;         //!<! e-Xi mass-ept spectra (Efficiency denominator)
  THnSparse* fHistoEleXiMassvsElePtMCS2;         //!<! e-Xi mass-ept spectra (Efficiency numerator)
  THnSparse* fHistoEleXiMassvsElePtMCGen2;         //!<! e-Xi mass-ept spectra (Efficiency denominator)
  TH2F* fHistoElePtMCS;         //!<! e spectra (Efficiency numerator)
  TH2F* fHistoElePtMCGen;         //!<! e spectra (Efficiency denominator)

  THnSparse* fHistoElePtvsEtaRS;         //!<! e spectra (right-sign)
  THnSparse* fHistoElePtvsEtaWS;         //!<! e spectra (wrong-sign)
  THnSparse* fHistoElePtvsEtaRSMix;         //!<! e spectra (right-sign, mix)
  THnSparse* fHistoElePtvsEtaWSMix;         //!<! e spectra (wrong-sign, mix)
  THnSparse* fHistoElePtvsEtaMCS;         //!<! e spectra efficiency numerator
  THnSparse* fHistoElePtvsEtaMCGen;         //!<! e spectra efficiency denominator

  THnSparse* fHistoElePtvsXiPtRS;         //!<! e-Xi spectra (right-sign)
  THnSparse* fHistoElePtvsXiPtWS;         //!<! e-Xi spectra (wrong-sign)
  THnSparse* fHistoElePtvsXiPtRSMix;         //!<! e-Xi spectra (right-sign, mix)
  THnSparse* fHistoElePtvsXiPtWSMix;         //!<! e-Xi spectra (wrong-sign, mix)
  THnSparse* fHistoElePtvsXiPtMCS;         //!<! e-Xi spectra efficiency numerator
  THnSparse* fHistoElePtvsXiPtvsXicPtMCS;         //!<! e-Xi spectra efficiency numerator
  THnSparse* fHistoElePtvsXiPtMCGen;         //!<! e-Xi spectra efficiency denominator
  THnSparse* fHistoElePtvsXiPtvsXicPtMCGen;         //!<! e-Xi spectra efficiency numerator
  THnSparse* fHistoElePtvsXiPtMCXicGen;         //!<! e-Xi spectra efficiency denominator

  THnSparse* fHistoElePtvsd0RS;         //!<! e pt-d0 spectra (right-sign)
  THnSparse* fHistoElePtvsd0WS;         //!<! e pt-d0 spectra (wrong-sign)
  THnSparse* fHistoElePtvsd0RSMix;         //!<! e pt-d0 spectra (right-sign, mix)
  THnSparse* fHistoElePtvsd0WSMix;         //!<! e pt-d0 spectra (wrong-sign, mix)
  THnSparse* fHistoElePtvsd0MCS;         //!<! e pt-d0 spectra (right-sign) 
  THnSparse* fHistoElePtvsd0PromptMCS;         //!<! e pt-d0 spectra (right-sign) 
  THnSparse* fHistoElePtvsd0BFeeddownMCS;         //!<! e pt-d0 spectra (right-sign) 

  TH1F* fHistoBachPt;      //!<! Bachelor pT histogram
  TH1F* fHistoBachPtMCS;      //!<! Bachelor pT histogram (efficiency numerator)
  TH1F* fHistoBachPtMCGen;      //!<! Bachelor pT histogram (efficiency denominator)
  TH1F* fHistod0Bach;      //!<! Bachelor d0 histogram
  TH2F* fHistoXiMassvsPt;     //!<! Xi mass vs pt histogram
  TH2F* fHistoXiMassvsPtMCS;     //!<! Xi mass vs pt histogram
  TH2F* fHistoXiMassvsPtMCGen;     //!<! Xi mass vs pt histogram
  TH2F* fHistoOmegaMassvsPt;     //!<! Omega mass vs pt histogram
  TH2F* fHistoElectronTPCPID;     //!<! TPC electron PID
  TH2F* fHistoElectronTOFPID;     //!<! TOF electron PID
  TH2F* fHistoElectronTPCSelPID;     //!<! TPC electron PID after selection
  TH2F* fHistoElectronTOFSelPID;     //!<! TOF electron PID after selection
  TH2F* fHistoElectronTPCPIDSelTOF;     //!<! TPC electron PID after TOF 3 sigma cut
  TH2F* fHistoElectronTOFPIDSelTPC;     //!<! TOF electron PID after TPC  cut
  TH2F* fHistoElectronTPCPIDSelTOFSmallEta;     //!<! TPC electron PID after TOF 3 sigma cut (|eta|<0.6)
  TH2F* fHistoElectronTPCPIDSelTOFLargeEta;     //!<! TPC electron PID after TOF 3 sigma cut (0.8>|eta|>0.6)
  TH2F* fHistoElectronTPCPIDSelTOFEtaDep[8];     //!<! TPC electron PID after TOF 3 sigma cut Eta dep
  TH1F* fHistoMassConversionsMin; //!<! electron-any mass
  TH1F* fHistoMassConversionsSameSignMin; //!<! electron-any mass
  THnSparse* fHistoElePtvsCutVarsRS[23];         //!<! e pt- cut variables (Right-sign)
  THnSparse* fHistoElePtvsCutVarsWS[23];         //!<! e pt- cut variables (Wrong-sign)
  THnSparse* fHistoElePtvsCutVarsMCS[23];         //!<! e pt- cut variables (MCS)
  TH2F* fHistoElectronQovPtvsPhi;     //!<! Electron phi distribution
  TH2F* fHistoXiQovPtvsPhi;     //!<! Xi phi distribution
  THnSparse* fHistoXicMCGen;         //!<! electron in mcArray
  THnSparse* fHistoXicMCGen1;         //!<! electron in mcArray
  THnSparse* fHistoXicMCGen2;         //!<! electron in mcArray
  THnSparse* fHistoXicMCS;         //!<! electron in mcArray
  THnSparse* fHistoXicMCS1;         //!<! electron in mcArray
  THnSparse* fHistoXicMCS2;         //!<! electron in mcArray
  THnSparse* fHistoXicElectronMCGen;         //!<! electron in mcArray
  THnSparse* fHistoXicElectronMCGen1;         //!<! electron in mcArray
  THnSparse* fHistoXicElectronMCGen2;         //!<! electron in mcArray
  THnSparse* fHistoXicElectronMCS;         //!<! electron in mcArray
  THnSparse* fHistoXicElectronMCS1;         //!<! electron in mcArray
  THnSparse* fHistoXicElectronMCS2;         //!<! electron in mcArray
  THnSparse* fHistoElectronMCGen;         //!<! electron in mcArray (only from charmed baryon)
  THnSparse* fHistoBottomElectronMCGen;         //!<! electron in mcArray (only from charmed baryon)
  THnSparse* fHistoCharmElectronMCGen;         //!<! electron in mcArray (only from charmed baryon)
  THnSparse* fHistoXiMCGen;         //!<! Xi in mcArray (only from charmed baryon)

	TH2F *fHistoLambdaPtvsDl; //!<! Lambda proper life time distribution
	TH2F *fHistoLambdaPtvsDlSide; //!<! Lambda proper life time distribution (sideband)
	TH2F *fHistoLambdaPtvsDlMCS; //!<! Lambda proper life time distribution (mc)
	TH2F *fHistoLambdaPtvsDR; //!<! Lambda decay length distribution
	TH2F *fHistoLambdaPtvsDRSide; //!<! Lambda decay length distribution (sideband)
	TH2F *fHistoLambdaPtvsDRMCS; //!<! Lambda decay length distribution (mc)

	TH2F *fHistoEleXiPtvsRapidityRS; //!<! e-Xi pT vs y
	TH2F *fHistoEleXiPtvsRapidityWS; //!<! e-Xi pT vs y
	TH2F *fHistoEleXiPtvsRapidityMCS; //!<! e-Xi pT vs y

	TH2D *fHistoResponseElePt; //!<! Response function electron pT <- True ept
	TH2D *fHistoResponseXiPt; //!<! Response function Lambda pT <- True ept
	TH2D *fHistoResponseEleXiPt; //!<! Response function e-Xi pT <- XicPt

  THnSparse* fHistoElectronPi0Total;         //!<! Number of electrons from pi0
  THnSparse* fHistoElectronPi0Tag;         //!<! Number of electrons from pi0 and have partner
  THnSparse* fHistoElectronEtaTotal;         //!<! Number of electrons from eta
  THnSparse* fHistoElectronEtaTag;         //!<! Number of electrons from eta and have partner


  AliNormalizationCounter *fCounter;//!<! Counter for normalization
	TH1F *fHistonEvtvsRunNumber;//!<!  evt vs runnumber
	TH1F *fHistonElevsRunNumber;//!<!  nele vs runnumber
	TH1F *fHistonXivsRunNumber;//!<!  nxi vs runnumber
	TH1F *fHistoMCEventType;//!<!  MC Event Type
	TH1F *fHistoMCXic0Decays;//!<!  MC Event Type
	TH1F *fHistoMCDeltaPhiccbar;//!<!  MC Event Type
	TH1F *fHistoMCNccbar;//!<!  MC Event Type

  // Store pointers to global tracks for pid and dca
  AliAODTrack **fGTI;                //! Array of pointers, just nicely sorted according to the id
  Int_t *fGTIndex;                //! Array of integers to keep the index of tpc only track
  const UShort_t  fTrackBuffSize;          //! Size of the above array, ~12000 for PbPb
  TH2D *fHistodPhiSdEtaSElectronProtonR125RS;//!<! dPhiS vs dEtaS R125 RS
  TH2D *fHistodPhiSdEtaSElectronProtonR125WS;//!<! dPhiS vs dEtaS R125 WS
  TH2D *fHistodPhiSdEtaSElectronProtonR125RSMix;//!<! dPhiS vs dEtaS R125 RS Mix
  TH2D *fHistodPhiSdEtaSElectronProtonR125WSMix;//!<! dPhiS vs dEtaS R125 WS Mix
  TH2D *fHistodPhiSdEtaSElectronPionR125RS;//!<! dPhiS vs dEtaS R125 RS
  TH2D *fHistodPhiSdEtaSElectronPionR125WS;//!<! dPhiS vs dEtaS R125 WS
  TH2D *fHistodPhiSdEtaSElectronPionR125RSMix;//!<! dPhiS vs dEtaS R125 RS Mix
  TH2D *fHistodPhiSdEtaSElectronPionR125WSMix;//!<! dPhiS vs dEtaS R125 WS Mix
  TH2D *fHistodPhiSdEtaSElectronBachelorR125RS;//!<! dPhiS vs dEtaS R125 RS
  TH2D *fHistodPhiSdEtaSElectronBachelorR125WS;//!<! dPhiS vs dEtaS R125 WS
  TH2D *fHistodPhiSdEtaSElectronBachelorR125RSMix;//!<! dPhiS vs dEtaS R125 RS Mix
  TH2D *fHistodPhiSdEtaSElectronBachelorR125WSMix;//!<! dPhiS vs dEtaS R125 WS Mix

  //Mixing
  Int_t fDoEventMixing; /// flag for event mixing
  Bool_t fMixWithoutConversionFlag; /// flag for mixing
  Int_t  fNumberOfEventsForMixing; /// maximum number of events to be used in event mixing
	Int_t fNzVtxBins;								/// number of z vrtx bins
	Double_t fZvtxBins[100];						// [fNzVtxBinsDim]
	Int_t fNCentBins;								/// number of centrality bins
	Double_t fCentBins[100];						// [fNCentBinsDim]
  Int_t  fNOfPools; /// number of pools
  TTree** fEventBuffer;   //!<! structure for event mixing
	TObjString *fEventInfo; /// unique event id for mixed event check
  TObjArray* fElectronTracks; /// array of e-compatible tracks
  TObjArray* fCascadeTracks1; /// array of xi+compatible tracks
  TObjArray* fCascadeTracks2; /// array of xi-compatible tracks
  TObjArray* fElectronCutVarsArray; /// array of RDHF cut information
  TObjArray* fCascadeCutVarsArray1; /// array of RDHF cut information
  TObjArray* fCascadeCutVarsArray2; /// array of RDHF cut information

  /// \cond CLASSIMP
  ClassDef(AliAnalysisTaskSEXic2eleXifromAODtracks,24); /// class for Xic->e Xi
  /// \endcond
};
#endif

