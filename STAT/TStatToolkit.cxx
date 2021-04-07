/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
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

//-----------------------------------------------------------------------------
/// \namespace TStatToolkit
/// \authors Marian Ivanov, Jens Wiechula, Ruben Shahoian,  Sebastian Lehner ...
/// Subset of  mathematical functions not included in the TMath
//-----------------------------------------------------------------------------

//
/////////////////////////////////////////////////////////////////////////
#include "TStopwatch.h"
#include "TStatToolkit.h"
#include "TTreeFormula.h"
#include "TLegend.h"
#include "TPRegexp.h"
#include "AliDrawStyle.h"
#include <stdlib.h>
#include <stdexcept>      // std::invalid_argument
using std::cout;
using std::cerr;
using std::endl;
using namespace std;

//_____________________________________________________________________________
void TStatToolkit::EvaluateUni(Int_t nvectors, Double_t *data, Double_t &mean, Double_t &sigma, Int_t hh) {
  //
  // Robust estimator in 1D case MI version - (faster than ROOT version)
  //
  // For the univariate case
  // estimates of location and scatter are returned in mean and sigma parameters
  // the algorithm works on the same principle as in multivariate case -
  // it finds a subset of size hh with smallest sigma, and then returns mean and
  // sigma of this subset
  //

  if (hh==0)
    hh=(nvectors+2)/2;
  Double_t faclts[]={2.6477,2.5092,2.3826,2.2662,2.1587,2.0589,1.9660,1.879,1.7973,1.7203,1.6473};
  Int_t *index=new Int_t[nvectors];
  TMath::Sort(nvectors, data, index, kFALSE);
  
  Int_t    nquant = TMath::Min(Int_t(Double_t(((hh*1./nvectors)-0.5)*40))+1, 11);
  Double_t factor = faclts[TMath::Max(0,nquant-1)];
  
  Double_t sumx  =0;
  Double_t sumx2 =0;
  Double_t bestmean  = 0; 
  Double_t bestsigma = (data[index[nvectors-1]]-data[index[0]]+1.);   // maximal possible sigma
  bestsigma *=bestsigma;

  for (Int_t i=0; i<hh; i++){
    sumx  += data[index[i]];
    sumx2 += data[index[i]]*data[index[i]];
  }
  
  Double_t norm = 1./Double_t(hh);
  Double_t norm2 = (hh-1)>0 ? 1./Double_t(hh-1):1;
  for (Int_t i=hh; i<nvectors; i++){
    Double_t cmean  = sumx*norm;
    Double_t csigma = (sumx2 - hh*cmean*cmean)*norm2;
    if (csigma<bestsigma){
      bestmean  = cmean;
      bestsigma = csigma;
    }
    
    sumx  += data[index[i]]-data[index[i-hh]];
    sumx2 += data[index[i]]*data[index[i]]-data[index[i-hh]]*data[index[i-hh]];
  }
  
  Double_t bstd=factor*TMath::Sqrt(TMath::Abs(bestsigma));
  mean  = bestmean;
  sigma = bstd;
  delete [] index;

}



void TStatToolkit::EvaluateUniExternal(Int_t nvectors, Double_t *data, Double_t &mean, Double_t &sigma, Int_t hh,  Float_t externalfactor) {
  // Modified version of ROOT robust EvaluateUni
  // robust estimator in 1D case MI version
  // added external factor to include precision of external measurement
  // 

  if (hh==0)
    hh=(nvectors+2)/2;
  Double_t faclts[]={2.6477,2.5092,2.3826,2.2662,2.1587,2.0589,1.9660,1.879,1.7973,1.7203,1.6473};
  Int_t *index=new Int_t[nvectors];
  TMath::Sort(nvectors, data, index, kFALSE);
  //
  Int_t    nquant = TMath::Min(Int_t(Double_t(((hh*1./nvectors)-0.5)*40))+1, 11);
  Double_t factor = faclts[0];
  if (nquant>0){
    // fix proper normalization - Anja
    factor = faclts[nquant-1];
  }

  //
  //
  Double_t sumx  =0;
  Double_t sumx2 =0;
  Double_t bestmean  = 0; 
  Double_t bestsigma = -1;
  for (Int_t i=0; i<hh; i++){
    sumx  += data[index[i]];
    sumx2 += data[index[i]]*data[index[i]];
  }
  //   
  Double_t kfactor = 2.*externalfactor - externalfactor*externalfactor;
  Double_t norm = 1./Double_t(hh);
  for (Int_t i=hh; i<nvectors; i++){
    Double_t cmean  = sumx*norm;
    Double_t csigma = (sumx2*norm - cmean*cmean*kfactor);
    if (csigma<bestsigma ||  bestsigma<0){
      bestmean  = cmean;
      bestsigma = csigma;
    }
    //
    //
    sumx  += data[index[i]]-data[index[i-hh]];
    sumx2 += data[index[i]]*data[index[i]]-data[index[i-hh]]*data[index[i-hh]];
  }
  
  Double_t bstd=factor*TMath::Sqrt(TMath::Abs(bestsigma));
  mean  = bestmean;
  sigma = bstd;
  delete [] index;
}


//_____________________________________________________________________________
Int_t TStatToolkit::Freq(Int_t n, const Int_t *inlist
                        , Int_t *outlist, Bool_t down)
{    
  //
  //  Sort eleements according occurancy 
  //  The size of output array has is 2*n 
  //

  Int_t * sindexS = new Int_t[n];     // temp array for sorting
  Int_t * sindexF = new Int_t[2*n];   
  for (Int_t i=0;i<n;i++) sindexS[i]=0;
  for (Int_t i=0;i<2*n;i++) sindexF[i]=0;
  //
  TMath::Sort(n,inlist, sindexS, down);  
  Int_t last      = inlist[sindexS[0]];
  Int_t val       = last;
  sindexF[0]      = 1;
  sindexF[0+n]    = last;
  Int_t countPos  = 0;
  //
  //  find frequency
  for(Int_t i=1;i<n; i++){
    val = inlist[sindexS[i]];
    if (last == val)   sindexF[countPos]++;
    else{      
      countPos++;
      sindexF[countPos+n] = val;
      sindexF[countPos]++;
      last =val;
    }
  }
  if (last==val) countPos++;
  // sort according frequency
  TMath::Sort(countPos, sindexF, sindexS, kTRUE);
  for (Int_t i=0;i<countPos;i++){
    outlist[2*i  ] = sindexF[sindexS[i]+n];
    outlist[2*i+1] = sindexF[sindexS[i]];
  }
  delete [] sindexS;
  delete [] sindexF;
  
  return countPos;

}



void TStatToolkit::MedianFilter(TH1 * his1D, Int_t nmedian){
  //
  // Algorithm to filter  histogram
  // author:  marian.ivanov@cern.ch
  // Details of algorithm:
  // http://en.wikipedia.org/w/index.php?title=Median_filter&oldid=582191524
  // Input parameters:
  //    his1D - input histogam - to be modiefied by Medianfilter
  //    nmendian - number of bins in median filter
  //
  Int_t nbins    = his1D->GetNbinsX();
  TVectorD vectorH(nbins);
  for (Int_t ibin=0; ibin<nbins; ibin++) vectorH[ibin]=his1D->GetBinContent(ibin+1);
  for (Int_t ibin=0; ibin<nbins; ibin++) {
    Int_t index0=ibin-nmedian;
    Int_t index1=ibin+nmedian;
    if (index0<0) {index1+=-index0; index0=0;}
    if (index1>=nbins) {index0-=index1-nbins+1; index1=nbins-1;}    
    Double_t value= TMath::Median(index1-index0,&(vectorH.GetMatrixArray()[index0]));
    his1D->SetBinContent(ibin+1, value);
  }  
}



Float_t TStatToolkit::GetCOG(const Short_t *arr, Int_t nBins, Float_t xMin, Float_t xMax, Float_t *rms, Float_t *sum)
{
    //
    //  calculate center of gravity rms and sum for array 'arr' with nBins an a x range xMin to xMax
    //  return COG; in case of failure return xMin
    //
    Float_t meanCOG = 0;
    Float_t rms2COG = 0;
    Float_t sumCOG  = 0;
    Int_t npoints   = 0;

    Float_t binWidth = (xMax-xMin)/(Float_t)nBins;

    for (Int_t ibin=0; ibin<nBins; ibin++){
	Float_t entriesI = (Float_t)arr[ibin];
	Double_t xcenter = xMin+(ibin+0.5)*binWidth;
	if ( entriesI>0 ){
	    meanCOG += xcenter*entriesI;
	    rms2COG += xcenter*entriesI*xcenter;
	    sumCOG  += entriesI;
	    npoints++;
	}
    }
    if ( sumCOG == 0 ) return xMin;
    meanCOG/=sumCOG;

    if ( rms ){
	rms2COG /=sumCOG;
	(*rms) = TMath::Sqrt(TMath::Abs(meanCOG*meanCOG-rms2COG));
	if ( npoints == 1 ) (*rms) = binWidth/TMath::Sqrt(12);
    }

    if ( sum )
        (*sum) = sumCOG;

    return meanCOG;
}



///////////////////////////////////////////////////////////////
//////////////         TEST functions /////////////////////////
///////////////////////////////////////////////////////////////





void TStatToolkit::TestGausFit(Int_t nhistos){
  //
  // Test performance of the parabolic - gaussian fit - compare it with 
  // ROOT gauss fit
  //  nhistos - number of histograms to be used for test
  //
  TTreeSRedirector *pcstream = new TTreeSRedirector("fitdebug.root","recreate");
  
  Float_t  *xTrue = new Float_t[nhistos];
  Float_t  *sTrue = new Float_t[nhistos];
  TVectorD **par1  = new TVectorD*[nhistos];
  TVectorD **par2  = new TVectorD*[nhistos];
  TMatrixD dummy(3,3);
  
  
  TH1F **h1f = new TH1F*[nhistos];
  TF1  *myg = new TF1("myg","gaus");
  TF1  *fit = new TF1("fit","gaus");
  gRandom->SetSeed(0);
  
  //init
  for (Int_t i=0;i<nhistos; i++){
    par1[i] = new TVectorD(3);
    par2[i] = new TVectorD(3);
    h1f[i]  = new TH1F(Form("h1f%d",i),Form("h1f%d",i),20,-10,10);
    xTrue[i]= gRandom->Rndm();
    gSystem->Sleep(2);
    sTrue[i]= .75+gRandom->Rndm()*.5;
    myg->SetParameters(1,xTrue[i],sTrue[i]);
    h1f[i]->FillRandom("myg");
  }
  
  TStopwatch s; 
  s.Start();
  //standard gaus fit
  for (Int_t i=0; i<nhistos; i++){
    h1f[i]->Fit(fit,"0q");
    (*par1[i])(0) = fit->GetParameter(0);
    (*par1[i])(1) = fit->GetParameter(1);
    (*par1[i])(2) = fit->GetParameter(2);
  }
  s.Stop();
  printf("Gaussian fit\t");
  s.Print();
  
  s.Start();
  //TStatToolkit gaus fit
  for (Int_t i=0; i<nhistos; i++){
    TStatToolkit::FitGaus(h1f[i]->GetArray()+1,h1f[i]->GetNbinsX(),h1f[i]->GetXaxis()->GetXmin(),h1f[i]->GetXaxis()->GetXmax(),par2[i],&dummy);
  }
  
  s.Stop();
  printf("Parabolic fit\t");
  s.Print();
  //write stream
  for (Int_t i=0;i<nhistos; i++){
    Float_t xt  = xTrue[i];
    Float_t st  = sTrue[i];
    (*pcstream)<<"data"
	       <<"xTrue="<<xt
	       <<"sTrue="<<st
	       <<"pg.="<<(par1[i])
	       <<"pa.="<<(par2[i])
	       <<"\n";
  }    
  //delete pointers
  for (Int_t i=0;i<nhistos; i++){
    delete par1[i];
    delete par2[i];
    delete h1f[i];
  }
  delete pcstream;
  delete []h1f;
  delete []xTrue;
  delete []sTrue;
  //
  delete []par1;
  delete []par2;

}



TGraph2D * TStatToolkit::MakeStat2D(TH3 * his, Int_t delta0, Int_t delta1, Int_t type){
  //
  //
  //
  // delta - number of bins to integrate
  // type - 0 - mean value

  TAxis * xaxis  = his->GetXaxis();
  TAxis * yaxis  = his->GetYaxis();
  //  TAxis * zaxis  = his->GetZaxis();
  Int_t   nbinx  = xaxis->GetNbins();
  Int_t   nbiny  = yaxis->GetNbins();
  char name[1000];
  Int_t icount=0;
  TGraph2D  *graph = new TGraph2D(nbinx*nbiny);
  TF1 f1("f1","gaus");
  for (Int_t ix=0; ix<nbinx;ix++)
    for (Int_t iy=0; iy<nbiny;iy++){
      Float_t xcenter = xaxis->GetBinCenter(ix); 
      Float_t ycenter = yaxis->GetBinCenter(iy); 
      snprintf(name,1000,"%s_%d_%d",his->GetName(), ix,iy);
      TH1 *projection = his->ProjectionZ(name,ix-delta0,ix+delta0,iy-delta1,iy+delta1);
      Float_t stat= 0;
      if (type==0) stat = projection->GetMean();
      if (type==1) stat = projection->GetRMS();
      if (type==2 || type==3){
	TVectorD vec(10);
	TStatToolkit::LTM((TH1F*)projection,&vec,0.7);
	if (type==2) stat= vec[1];
	if (type==3) stat= vec[0];	
      }
      if (type==4|| type==5){
	projection->Fit(&f1);
	if (type==4) stat= f1.GetParameter(1);
	if (type==5) stat= f1.GetParameter(2);
      }
      //printf("%d\t%f\t%f\t%f\n", icount,xcenter, ycenter, stat);
      graph->SetPoint(icount,xcenter, ycenter, stat);
      icount++;
    }
  return graph;
}

TGraphErrors * TStatToolkit::MakeStat1D(TH2 * his, Int_t deltaBin, Double_t fraction, Int_t returnType, Int_t markerStyle, Int_t markerColor){
  //
  // function to retrieve the "mean and RMS estimate" of 2D histograms
  //     
  // Robust statistic to estimate properties of the distribution
  // See http://en.wikipedia.org/wiki/Trimmed_estimator
  //
  // deltaBin - number of bins to integrate (bin+-deltaBin)
  // fraction - fraction of values for the LTM and for the gauss fit
  // returnType - 
  //        0 - mean value
  //        1 - RMS
  //        2 - LTM mean
  //        3 - LTM sigma
  //        4 - Gaus fit mean  - on LTM range
  //        5 - Gaus fit sigma - on LTM  range
  //        6 - Robust bin median
  //        7 - Gaus fit around maxium of the distribution, fraction is +- relative range of full y scale
  // 
  TAxis * xaxis  = his->GetXaxis();
  Int_t   nbinx  = xaxis->GetNbins();
  char name[1000];
  Int_t icount=0;
  //
  TVectorD vecX(nbinx);
  TVectorD vecXErr(nbinx);
  TVectorD vecY(nbinx);
  TVectorD vecYErr(nbinx);
  //
  TF1 f1("f1","gaus");
  TVectorD vecLTM(10);

  for (Int_t jx=1; jx<=nbinx;jx++){
    Int_t ix=jx-1;
    Float_t xcenter = xaxis->GetBinCenter(jx); 
    snprintf(name,1000,"%s_%d",his->GetName(), ix);
    TH1 *projection = his->ProjectionY(name,TMath::Max(jx-deltaBin,1),TMath::Min(jx+deltaBin,nbinx));
    Double_t stat= 0;
    Double_t err =0;
    if (projection->Integral()==0) {
      vecX[icount] = xcenter;
      vecY[icount] = stat;
      vecYErr[icount] = err;
      icount++;
      delete projection;
      continue;
    }

    TStatToolkit::LTMHisto((TH1F*)projection,vecLTM,fraction);  
    //
    if (returnType==0) {
      stat = projection->GetMean();
      err  = projection->GetMeanError();
    }
    else if (returnType==1) {
      stat = projection->GetRMS();
      err = projection->GetRMSError();
    }
    else if (returnType==2 || returnType==3){
      if (returnType==2) {stat= vecLTM[1];  err =projection->GetRMSError();}
	if (returnType==3) {stat= vecLTM[2];	 err =projection->GetRMSError();}
    }
    else if (returnType==4|| returnType==5){
      f1.SetParameters(vecLTM[0], vecLTM[1], vecLTM[2]+0.05);
      projection->Fit(&f1,"QN","QN", vecLTM[7]-vecLTM[2], vecLTM[8]+vecLTM[2]);
      if (returnType==4) {
	stat= f1.GetParameter(1);
	err=f1.GetParError(1);
      }
      if (returnType==5) {
	stat= f1.GetParameter(2);
	err=f1.GetParError(2);
      }
    }
    else if (returnType==6) {
      stat=RobustBinMedian(projection,fraction);
    }
    else if (returnType==7) {
      const Int_t    maxBin = projection->GetMaximumBin();
      const Double_t max    = projection->GetXaxis()->GetBinCenter(maxBin);
      const Double_t range  = fraction*(projection->GetXaxis()->GetXmax()-projection->GetXaxis()->GetXmin());
      f1.SetParameters(projection->GetMaximum(),
		       projection->GetMean(),
		       projection->GetRMS());
      f1.SetRange(max-range, max+range);
      projection->Fit(&f1,"QN","QN", max-range, max+range);
      stat= f1.GetParameter(1);
      err=f1.GetParError(1);
    }

    vecX[icount]=xcenter;
    vecY[icount]=stat;
    vecYErr[icount]=err;
    icount++;
    delete projection;
  }
  TGraphErrors  *graph = new TGraphErrors(icount,vecX.GetMatrixArray(), vecY.GetMatrixArray(),0, vecYErr.GetMatrixArray());
  graph->SetMarkerStyle(markerStyle);
  graph->SetMarkerColor(markerColor);
  return graph;
}

Double_t TStatToolkit::RobustBinMedian(TH1* hist, Double_t fractionCut/*=1.*/)
{
  // Robust median with average from neighbouring bins
  const Int_t nbins1D=hist->GetNbinsX();
  Double_t binMedian=0;
  Double_t limits[2]={hist->GetBinCenter(1), hist->GetBinCenter(nbins1D)};
  (void) limits[0];

  Double_t* integral=hist->GetIntegral();
  for (Int_t i=1; i<nbins1D-1; i++){
    if (integral[i-1]<0.5 && integral[i]>=0.5){
      if (hist->GetBinContent(i-1)+hist->GetBinContent(i)>0){
        binMedian=hist->GetBinCenter(i);
        Double_t dIdx=-(integral[i-1]-integral[i]);
        Double_t dx=(0.5+(0.5-integral[i])/dIdx)*hist->GetBinWidth(i);
        binMedian+=dx;
      }
    }
    if (integral[i-1]<fractionCut && integral[i]>=fractionCut){
      limits[0]=hist->GetBinCenter(i-1)-hist->GetBinWidth(i);
    }
    if (integral[i]<1-fractionCut && integral[i+1]>=1-fractionCut){
      limits[1]=hist->GetBinCenter(i+1)+hist->GetBinWidth(i);
    }
  }

  return binMedian;
}


TString* TStatToolkit::FitPlane(TTree *tree, const char* drawCommand, const char* formula, const char* cuts, Double_t & chi2, Int_t &npoints, TVectorD &fitParam, TMatrixD &covMatrix, Float_t frac, Int_t start, Int_t stop,Bool_t fix0){
   //
   // fit an arbitrary function, specified by formula into the data, specified by drawCommand and cuts
   // returns chi2, fitParam and covMatrix
   // returns TString with fitted formula
   //

   TString formulaStr(formula); 
   TString drawStr(drawCommand);
   TString cutStr(cuts);
   TString ferr("1");

   TString strVal(drawCommand);
   if (strVal.Contains(":")){
     TObjArray* valTokens = strVal.Tokenize(":");
     drawStr = valTokens->At(0)->GetName();
     ferr       = valTokens->At(1)->GetName();     
     delete valTokens;
   }

      
   formulaStr.ReplaceAll("++", "~");
   TObjArray* formulaTokens = formulaStr.Tokenize("~"); 
   Int_t dim = formulaTokens->GetEntriesFast();
   
   fitParam.ResizeTo(dim);
   covMatrix.ResizeTo(dim,dim);
   
   TLinearFitter* fitter = new TLinearFitter(dim+1, Form("hyp%d",dim));
   fitter->StoreData(kTRUE);   
   fitter->ClearPoints();
   
   Int_t entries = tree->Draw(drawStr.Data(), cutStr.Data(), "goff",  stop-start, start);
   if (entries == -1) {
     delete formulaTokens;
     return new TString(TString::Format("ERROR expr: %s\t%s\tEntries==0",drawStr.Data(),cutStr.Data()));
   }
   Double_t **values = new Double_t*[dim+1] ;
   for (Int_t i=0; i<dim+1; i++) values[i]=NULL; 
   //
   entries = tree->Draw(ferr.Data(), cutStr.Data(), "goff",  stop-start, start);
   if (entries == -1) {
     delete formulaTokens;
     delete []values;
     return new TString(TString::Format("ERROR error part: %s\t%s\tEntries==0",ferr.Data(),cutStr.Data()));
   }
   Double_t *errors = new Double_t[entries];
   memcpy(errors,  tree->GetV1(), entries*sizeof(Double_t));
   
   for (Int_t i = 0; i < dim + 1; i++){
      Int_t centries = 0;
      if (i < dim) centries = tree->Draw(((TObjString*)formulaTokens->At(i))->GetName(), cutStr.Data(), "goff", stop-start,start);
      else  centries = tree->Draw(drawStr.Data(), cutStr.Data(), "goff", stop-start,start);
      
      if (entries != centries) {
	delete []errors;
	delete []values;
	return new TString(TString::Format("ERROR: %s\t%s\tEntries==%d\tEntries2=%d\n",drawStr.Data(),cutStr.Data(),entries,centries));
      }
      values[i] = new Double_t[entries];
      memcpy(values[i],  tree->GetV1(), entries*sizeof(Double_t)); 
   }
   
   // add points to the fitter
   for (Int_t i = 0; i < entries; i++){
      Double_t x[1000];
      for (Int_t j=0; j<dim;j++) x[j]=values[j][i];
      fitter->AddPoint(x, values[dim][i], errors[i]);
   }

   fitter->Eval();
   if (frac>0.5 && frac<1){
     fitter->EvalRobust(frac);
   }else{
     if (fix0) {
       fitter->FixParameter(0,0);
       fitter->Eval();     
     }
   }
   fitter->GetParameters(fitParam);
   fitter->GetCovarianceMatrix(covMatrix);
   chi2 = fitter->GetChisquare();
   npoints = entries;   
   TString *preturnFormula = new TString(Form("( %f+",fitParam[0])), &returnFormula = *preturnFormula; 
   
   for (Int_t iparam = 0; iparam < dim; iparam++) {
     returnFormula.Append(Form("%s*(%f)",((TObjString*)formulaTokens->At(iparam))->GetName(),fitParam[iparam+1]));
     if (iparam < dim-1) returnFormula.Append("+");
   }
   returnFormula.Append(" )");
   
   
   for (Int_t j=0; j<dim+1;j++) delete [] values[j];


   delete formulaTokens;
   delete fitter;
   delete[] values;
   delete[] errors;
   return preturnFormula;
}

TString* TStatToolkit::FitPlaneConstrain(TTree *tree, const char* drawCommand, const char* formula, const char* cuts, Double_t & chi2, Int_t &npoints, TVectorD &fitParam, TMatrixD &covMatrix, Float_t frac, Int_t start, Int_t stop,Double_t constrain){
   //
   // fit an arbitrary function, specified by formula into the data, specified by drawCommand and cuts
   // returns chi2, fitParam and covMatrix
   // returns TString with fitted formula
   //

   TString formulaStr(formula); 
   TString drawStr(drawCommand);
   TString cutStr(cuts);
   TString ferr("1");

   TString strVal(drawCommand);
   if (strVal.Contains(":")){
     TObjArray* valTokens = strVal.Tokenize(":");
     drawStr = valTokens->At(0)->GetName();
     ferr       = valTokens->At(1)->GetName();     
     delete valTokens;
   }

      
   formulaStr.ReplaceAll("++", "~");
   TObjArray* formulaTokens = formulaStr.Tokenize("~"); 
   Int_t dim = formulaTokens->GetEntriesFast();
   
   fitParam.ResizeTo(dim);
   covMatrix.ResizeTo(dim,dim);
   
   TLinearFitter* fitter = new TLinearFitter(dim+1, Form("hyp%d",dim));
   fitter->StoreData(kTRUE);   
   fitter->ClearPoints();
   
   Int_t entries = tree->Draw(drawStr.Data(), cutStr.Data(), "goff",  stop-start, start);
   if (entries == -1) {
     delete formulaTokens;
     return new TString("An ERROR has occured during fitting!");
   }
   Double_t **values = new Double_t*[dim+1] ; 
   for (Int_t i=0; i<dim+1; i++) values[i]=NULL; 
   //
   entries = tree->Draw(ferr.Data(), cutStr.Data(), "goff",  stop-start, start);
   if (entries == -1) {
     delete formulaTokens;
     delete [] values;
     return new TString("An ERROR has occured during fitting!");
   }
   Double_t *errors = new Double_t[entries];
   memcpy(errors,  tree->GetV1(), entries*sizeof(Double_t));
   
   for (Int_t i = 0; i < dim + 1; i++){
      Int_t centries = 0;
      if (i < dim) centries = tree->Draw(((TObjString*)formulaTokens->At(i))->GetName(), cutStr.Data(), "goff", stop-start,start);
      else  centries = tree->Draw(drawStr.Data(), cutStr.Data(), "goff", stop-start,start);
      
      if (entries != centries) {
	delete []errors;
	delete []values;
	delete formulaTokens;
	return new TString("An ERROR has occured during fitting!");
      }
      values[i] = new Double_t[entries];
      memcpy(values[i],  tree->GetV1(), entries*sizeof(Double_t)); 
   }
   
   // add points to the fitter
   for (Int_t i = 0; i < entries; i++){
      Double_t x[1000];
      for (Int_t j=0; j<dim;j++) x[j]=values[j][i];
      fitter->AddPoint(x, values[dim][i], errors[i]);
   }
   if (constrain>0){
     for (Int_t i = 0; i < dim; i++){
       Double_t x[1000];
       for (Int_t j=0; j<dim;j++) if (i!=j) x[j]=0;
       x[i]=1.;
       fitter->AddPoint(x, 0, constrain);
     }
   }


   fitter->Eval();
   if (frac>0.5 && frac<1){
     fitter->EvalRobust(frac);   
   }
   fitter->GetParameters(fitParam);
   fitter->GetCovarianceMatrix(covMatrix);
   chi2 = fitter->GetChisquare();
   npoints = entries;
   
   TString *preturnFormula = new TString(Form("( %f+",fitParam[0])), &returnFormula = *preturnFormula; 
   
   for (Int_t iparam = 0; iparam < dim; iparam++) {
     returnFormula.Append(Form("%s*(%f)",((TObjString*)formulaTokens->At(iparam))->GetName(),fitParam[iparam+1]));
     if (iparam < dim-1) returnFormula.Append("+");
   }
   returnFormula.Append(" )");
   
   for (Int_t j=0; j<dim+1;j++) delete [] values[j];
   


   delete formulaTokens;
   delete fitter;
   delete[] values;
   delete[] errors;
   return preturnFormula;
}



TString* TStatToolkit::FitPlaneFixed(TTree *tree, const char* drawCommand, const char* formula, const char* cuts, Double_t & chi2, Int_t &npoints, TVectorD &fitParam, TMatrixD &covMatrix, Float_t frac, Int_t start, Int_t stop){
   //
   // fit an arbitrary function, specified by formula into the data, specified by drawCommand and cuts
   // returns chi2, fitParam and covMatrix
   // returns TString with fitted formula
   //

   TString formulaStr(formula); 
   TString drawStr(drawCommand);
   TString cutStr(cuts);
   TString ferr("1");

   TString strVal(drawCommand);
   if (strVal.Contains(":")){
     TObjArray* valTokens = strVal.Tokenize(":");
     drawStr = valTokens->At(0)->GetName();
     ferr       = valTokens->At(1)->GetName();
     delete valTokens;
   }

      
   formulaStr.ReplaceAll("++", "~");
   TObjArray* formulaTokens = formulaStr.Tokenize("~"); 
   Int_t dim = formulaTokens->GetEntriesFast();
   
   fitParam.ResizeTo(dim);
   covMatrix.ResizeTo(dim,dim);
   TString fitString="x0";
   for (Int_t i=1; i<dim; i++) fitString+=Form("++x%d",i);     
   TLinearFitter* fitter = new TLinearFitter(dim, fitString.Data());
   fitter->StoreData(kTRUE);   
   fitter->ClearPoints();
   
   Int_t entries = tree->Draw(drawStr.Data(), cutStr.Data(), "goff",  stop-start, start);
   if (entries == -1) {
     delete formulaTokens;
     return new TString("An ERROR has occured during fitting!");
   }
   Double_t **values = new Double_t*[dim+1] ; 
   for (Int_t i=0; i<dim+1; i++) values[i]=NULL; 
   //
   entries = tree->Draw(ferr.Data(), cutStr.Data(), "goff",  stop-start, start);
   if (entries == -1) {
     delete []values;
     delete formulaTokens;
     return new TString("An ERROR has occured during fitting!");
   }
   Double_t *errors = new Double_t[entries];
   memcpy(errors,  tree->GetV1(), entries*sizeof(Double_t));
   
   for (Int_t i = 0; i < dim + 1; i++){
      Int_t centries = 0;
      if (i < dim) centries = tree->Draw(((TObjString*)formulaTokens->At(i))->GetName(), cutStr.Data(), "goff", stop-start,start);
      else  centries = tree->Draw(drawStr.Data(), cutStr.Data(), "goff", stop-start,start);
      
      if (entries != centries) {
	delete []errors;
	delete []values;
	delete formulaTokens;
	return new TString("An ERROR has occured during fitting!");
      }
      values[i] = new Double_t[entries];
      memcpy(values[i],  tree->GetV1(), entries*sizeof(Double_t)); 
   }
   
   // add points to the fitter
   for (Int_t i = 0; i < entries; i++){
      Double_t x[1000];
      for (Int_t j=0; j<dim;j++) x[j]=values[j][i];
      fitter->AddPoint(x, values[dim][i], errors[i]);
   }

   fitter->Eval();
   if (frac>0.5 && frac<1){
     fitter->EvalRobust(frac);
   }
   fitter->GetParameters(fitParam);
   fitter->GetCovarianceMatrix(covMatrix);
   chi2 = fitter->GetChisquare();
   npoints = entries;
   
   TString *preturnFormula = new TString("("), &returnFormula = *preturnFormula; 
   
   for (Int_t iparam = 0; iparam < dim; iparam++) {
     returnFormula.Append(Form("%s*(%f)",((TObjString*)formulaTokens->At(iparam))->GetName(),fitParam[iparam]));
     if (iparam < dim-1) returnFormula.Append("+");
   }
   returnFormula.Append(" )");
   
   
   for (Int_t j=0; j<dim+1;j++) delete [] values[j];
   
   delete formulaTokens;
   delete fitter;
   delete[] values;
   delete[] errors;
   return preturnFormula;
}





Int_t TStatToolkit::GetFitIndex(const TString fString, const TString subString){
  //
  // fitString - ++ separated list of fits
  // substring - ++ separated list of the requiered substrings
  //
  // return the last occurance of substring in fit string
  // 
  TObjArray *arrFit = fString.Tokenize("++");
  TObjArray *arrSub = subString.Tokenize("++");
  Int_t index=-1;
  for (Int_t i=0; i<arrFit->GetEntries(); i++){
    Bool_t isOK=kTRUE;
    TString str =arrFit->At(i)->GetName();
    for (Int_t isub=0; isub<arrSub->GetEntries(); isub++){
      if (str.Contains(arrSub->At(isub)->GetName())==0) isOK=kFALSE;
    }
    if (isOK) index=i;
  }
  delete arrFit;
  delete arrSub;
  return index;
}


TString  TStatToolkit::FilterFit(const TString &input, const TString filter, TVectorD &param, TMatrixD & covar){
  //
  // Filter fit expression make sub-fit
  //
  TObjArray *array0= input.Tokenize("++");
  TObjArray *array1= filter.Tokenize("++");
  //TString *presult=new TString("(0");
  TString result="(0.0";
  for (Int_t i=0; i<array0->GetEntries(); i++){
    Bool_t isOK=kTRUE;
    TString str(array0->At(i)->GetName());
    for (Int_t j=0; j<array1->GetEntries(); j++){
      if (str.Contains(array1->At(j)->GetName())==0) isOK=kFALSE;      
    }
    if (isOK) {
      result+="+"+str;
      result+=Form("*(%f)",param[i+1]);
      printf("%f\t%f\t%s\n",param[i+1], TMath::Sqrt(covar(i+1,i+1)),str.Data());    
    }
  }
  result+="-0.)";
  delete array0;
  delete array1;
  return result;
}

void TStatToolkit::Update1D(Double_t delta, Double_t sigma, Int_t s1, TMatrixD &vecXk, TMatrixD &covXk){
  //
  // Update parameters and covariance - with one measurement
  // Input:
  // vecXk - input vector - Updated in function 
  // covXk - covariance matrix - Updated in function
  // delta, sigma, s1 - new measurement, rms of new measurement and the index of measurement
  const Int_t knMeas=1;
  Int_t knElem=vecXk.GetNrows();
 
  TMatrixD mat1(knElem,knElem);            // update covariance matrix
  TMatrixD matHk(1,knElem);        // vector to mesurement
  TMatrixD vecYk(knMeas,1);        // Innovation or measurement residual
  TMatrixD matHkT(knElem,knMeas);  // helper matrix Hk transpose
  TMatrixD matSk(knMeas,knMeas);   // Innovation (or residual) covariance
  TMatrixD matKk(knElem,knMeas);   // Optimal Kalman gain
  TMatrixD covXk2(knElem,knElem);  // helper matrix
  TMatrixD covXk3(knElem,knElem);  // helper matrix
  TMatrixD vecZk(1,1);
  TMatrixD measR(1,1);
  vecZk(0,0)=delta;
  measR(0,0)=sigma*sigma;
  //
  // reset matHk
  for (Int_t iel=0;iel<knElem;iel++) 
    for (Int_t ip=0;ip<knMeas;ip++) matHk(ip,iel)=0; 
  //mat1
  for (Int_t iel=0;iel<knElem;iel++) {
    for (Int_t jel=0;jel<knElem;jel++) mat1(iel,jel)=0;
    mat1(iel,iel)=1;
  }
  //
  matHk(0, s1)=1;
  vecYk = vecZk-matHk*vecXk;               // Innovation or measurement residual
  matHkT=matHk.T(); matHk.T();
  matSk = (matHk*(covXk*matHkT))+measR;    // Innovation (or residual) covariance
  matSk.Invert();
  matKk = (covXk*matHkT)*matSk;            //  Optimal Kalman gain
  vecXk += matKk*vecYk;                    //  updated vector 
  covXk2= (mat1-(matKk*matHk));
  covXk3 =  covXk2*covXk;          
  covXk = covXk3;  
  Int_t nrows=covXk3.GetNrows();
  
  for (Int_t irow=0; irow<nrows; irow++)
    for (Int_t icol=0; icol<nrows; icol++){
      // rounding problems - make matrix again symteric
      covXk(irow,icol)=(covXk3(irow,icol)+covXk3(icol,irow))*0.5; 
    }
}



void   TStatToolkit::Constrain1D(const TString &input, const TString filter, TVectorD &param, TMatrixD & covar, Double_t mean, Double_t sigma){
  //
  // constrain linear fit
  // input  - string description of fit function
  // filter - string filter to select sub fits
  // param,covar - parameters and covariance matrix of the fit
  // mean,sigma  - new measurement uning which the fit is updated
  //
  
  TObjArray *array0= input.Tokenize("++");
  TObjArray *array1= filter.Tokenize("++");
  TMatrixD paramM(param.GetNrows(),1);
  for (Int_t i=0; i<=array0->GetEntries(); i++){paramM(i,0)=param(i);}
  
  if (filter.Length()==0){
    TStatToolkit::Update1D(mean, sigma, 0, paramM, covar);//
  }else{  
    for (Int_t i=0; i<array0->GetEntries(); i++){
      Bool_t isOK=kTRUE;
      TString str(array0->At(i)->GetName());
      for (Int_t j=0; j<array1->GetEntries(); j++){
	if (str.Contains(array1->At(j)->GetName())==0) isOK=kFALSE;      
      }
      if (isOK) {
	TStatToolkit::Update1D(mean, sigma, i+1, paramM, covar);//
      }
    }
  }
  for (Int_t i=0; i<=array0->GetEntries(); i++){
    param(i)=paramM(i,0);
  }
  delete array0;
  delete array1;
}

TString  TStatToolkit::MakeFitString(const TString &input, const TVectorD &param, const TMatrixD & covar, Bool_t verbose){
  //
  //
  //
  TObjArray *array0= input.Tokenize("++");
  TString result=Form("(%f",param[0]);
  printf("%f\t%f\t\n", param[0], TMath::Sqrt(covar(0,0))); 
  for (Int_t i=0; i<array0->GetEntries(); i++){
    TString str(array0->At(i)->GetName());
    result+="+"+str;
    result+=Form("*(%f)",param[i+1]);
    if (verbose) printf("%f\t%f\t%s\n", param[i+1], TMath::Sqrt(covar(i+1,i+1)),str.Data());    
  }
  result+="-0.)";
  delete array0;
  return result;
}

TGraphErrors * TStatToolkit::MakeGraphErrors(TTree * tree, const char * expr, const char * cut,  Int_t mstyle, Int_t mcolor, Float_t msize, Float_t offset, Int_t drawEntries, Int_t firstEntry){
  //
  // Query a graph errors
  // return TGraphErrors specified by expr and cut 
  // Example  usage TStatToolkit::MakeGraphError(tree,"Y:X:ErrY","X>0", 25,2,0.4)
  // tree   - tree with variable
  // expr   - examp 
  const Int_t entries =  tree->Draw(expr,cut,"goff",drawEntries,firstEntry);
 
  if (entries<=0) {
    ::Error("TStatToolkit::MakeGraphError","Empty or Not valid expression (%s) or cut *%s)", expr,cut);
    return 0;
  }
  if (  tree->GetV2()==0){
    ::Error("TStatToolkit::MakeGraphError","Not valid expression (%s) ", expr);
    return 0;
  }
  TGraphErrors * graph=0;
  if ( tree->GetV3()!=0){
    graph = new TGraphErrors (entries, tree->GetV2(),tree->GetV1(),0,tree->GetV3());
  }else{
    graph = new TGraphErrors (entries, tree->GetV2(),tree->GetV1(),0,0);
  }

  graph->SetMarkerStyle(mstyle); 
  graph->SetMarkerColor(mcolor);
  graph->SetLineColor(mcolor);
  graph->SetTitle(expr);
  TString chstring(expr);
  TObjArray *charray = chstring.Tokenize(":");
  graph->GetXaxis()->SetTitle(charray->At(1)->GetName());
  graph->GetYaxis()->SetTitle(charray->At(0)->GetName());
  THashList * metaData = (THashList*) tree->GetUserInfo()->FindObject("metaTable");
  if ( metaData != NULL){
    TNamed *nmdTitle0 = TStatToolkit::GetMetadata(tree,Form("%s.Title",charray->At(0)->GetName()));
    TNamed *nmdTitle1 = TStatToolkit::GetMetadata(tree,Form("%s.Title",charray->At(1)->GetName()));
    TNamed *nmdYAxis  = TStatToolkit::GetMetadata(tree,Form("%s.AxisTitle",charray->At(0)->GetName()));
    TNamed *nmdXAxis  = TStatToolkit::GetMetadata(tree,Form("%s.AxisTitle",charray->At(1)->GetName())); 
    //
    TString grTitle=charray->At(0)->GetName();
    if (nmdTitle0)  grTitle=nmdTitle0->GetTitle();
    if (nmdTitle1)  {
      grTitle+=":";
      grTitle+=nmdTitle1->GetTitle();
    }else{
      grTitle+=":";
      grTitle+=charray->At(1)->GetName();
    }
    if (nmdYAxis) {graph->GetYaxis()->SetTitle(nmdYAxis->GetTitle());}
    if (nmdXAxis) {graph->GetXaxis()->SetTitle(nmdXAxis->GetTitle());}  
    graph->SetTitle(grTitle.Data());
  }  
  delete charray;
  if (msize>0) graph->SetMarkerSize(msize);
  for(Int_t i=0;i<graph->GetN();i++) graph->GetX()[i]+=offset;
  //
  if (tree->GetVar(1)->IsString()){ // use string for axis description
    TAxis * axis = tree->GetHistogram()->GetXaxis();
    axis->Copy(*(graph->GetXaxis()));
  }
  if (tree->GetVar(0)->IsString()){
    TAxis * axis = tree->GetHistogram()->GetYaxis();
    axis->Copy(*(graph->GetYaxis()));
  }
  graph->Sort();
  return graph;
  
}

/// AddMetadata to the input tree - see https://alice.its.cern.ch/jira/browse/ATO-290
/// \param tree         - input tree
/// \param varTagName   - tag to register
/// \param varTagValue  - value
/// \return             - return has list
/// Currently supported metadata
///* Drawing  :
///  * <varName>.AxisTitle
///  *<varName>.Legend
///  * <varname>.Color
///  * <varname>.MarkerStyle
///* This metadata than can be used by the TStatToolkit
///  * TStatToolkit::MakeGraphSparse
///  * TStatToolkit::MakeGraphErrors
///
THashList*  TStatToolkit::AddMetadata(TTree* tree, const char *varTagName,const char *varTagValue){
  if (!tree) return NULL;
  THashList * metaData = (THashList*) tree->GetUserInfo()->FindObject("metaTable");
  if (metaData == NULL){  
    metaData=new THashList;
    metaData->SetName("metaTable");
    tree->GetUserInfo()->AddLast(metaData);
  } 
  if (varTagName!=NULL && varTagValue!=NULL){
    TNamed * named = TStatToolkit::GetMetadata(tree, varTagName,NULL,kTRUE);
    if (named==NULL){
      metaData->AddLast(new TNamed(varTagName,varTagValue));
    }else{
      named->SetTitle(varTagValue);
    }
  }
  return metaData;
}

/// Get metadata description
/// In case metadata contains friend part - friend path os returned as prefix
/// Metadata are supposed to be added into tree using TStatToolkit::AddMetadata() function
/// \param tree          - input tree
/// \param varTagName    - tag name
/// \param prefix        - friend prefix in case metadata are in friend tree
/// \param fullMatch     - request full match in varTagName (to enable different metadata for tree and friend tree)
/// \return              - metadata description
/// TODO: too many string operations - to be speed up using char array arithmetic
TNamed* TStatToolkit::GetMetadata(TTree* tree, const char *varTagName, TString * prefix,Bool_t fullMatch){
  //

  if (!tree) return 0;
  TTree * treeMeta=tree;

  THashList * metaData = (THashList*) treeMeta->GetUserInfo()->FindObject("metaTable");
  if (metaData == NULL){
    metaData=new THashList;
    metaData->SetName("metaTable");
    tree->GetUserInfo()->AddLast(metaData);
    return 0;
  }
  TNamed * named = (TNamed*)metaData->FindObject(varTagName);
  if (named || fullMatch) return named;

  TString metaName(varTagName);
  Int_t nDots= metaName.CountChar('.');
  if (prefix!=NULL) *prefix="";
  if (nDots>1){ //check if friend name expansion needed
    while (nDots>1){
      TList *fList= treeMeta->GetListOfFriends();
      if (fList!=NULL){
        Int_t nFriends= fList->GetEntries();
        for (Int_t kf=0; kf<nFriends; kf++){
          TPRegexp regFriend(TString::Format("^%s.",fList->At(kf)->GetName()).Data());
          if (metaName.Contains(regFriend)){
            treeMeta=treeMeta->GetFriend(fList->At(kf)->GetName());
            regFriend.Substitute(metaName,"");
            if (prefix!=NULL){
              (*prefix)+=fList->At(kf)->GetName();
              // (*prefix)+=""; /// FIX use prefix as it is
            }
          }
        }
      }
      if (nDots == metaName.CountChar('.')) break;
      nDots=metaName.CountChar('.');
    }
  }

  named = (TNamed*)metaData->FindObject(metaName.Data());
  return named;

}


TGraph * TStatToolkit::MakeGraphSparse(TTree * tree, const char * expr, const char * cut, Int_t mstyle, Int_t mcolor, Float_t msize, Float_t offset){
  //
  // Make a sparse draw of the variables
  // Format of expr : Var:Run or Var:Run:ErrorY or Var:Run:ErrorY:ErrorX
  // offset : points can slightly be shifted in x for better visibility with more graphs
  //
  // Patrick Reichelt and Marian Ivanov
  // maintained and updated by Marian Ivanov
  const Int_t entries = tree->Draw(expr,cut,"goff");
  if (entries<=0) {
    ::Error("TStatToolkit::MakeGraphSparse","Empty or Not valid expression (%s) or cut (%s)", expr, cut);
    return 0;
  }

  Double_t *graphY, *graphX;
  graphY = tree->GetV1();
  graphX = tree->GetV2();

  // sort according to run number
  Int_t *index = new Int_t[entries*4];
  TMath::Sort(entries,graphX,index,kFALSE);

  // define arrays for the new graph
  Double_t *unsortedX = new Double_t[entries];
  Int_t *runNumber = new Int_t[entries];
  Double_t count = 0.5;

  // evaluate arrays for the new graph according to the run-number
  Int_t icount=0;
  //first entry
  unsortedX[index[0]] = count;
  runNumber[0] = graphX[index[0]];
  // loop the rest of entries
  for(Int_t i=1;i<entries;i++)
  {
    if(graphX[index[i]]==graphX[index[i-1]])
      unsortedX[index[i]] = count;
    else if(graphX[index[i]]!=graphX[index[i-1]]){
      count++;
      icount++;
      unsortedX[index[i]] = count;
      runNumber[icount]=graphX[index[i]];
    }
  }

  // count the number of xbins (run-wise) for the new graph
  const Int_t newNbins = int(count+0.5);
  Double_t *newBins = new Double_t[newNbins+1];
  for(Int_t i=0; i<=count+1;i++){
    newBins[i] = i;
  }

  // define and fill the new graph
  TGraph *graphNew = 0;
  if (tree->GetV3()) {
    if (tree->GetV4()) {
      graphNew = new TGraphErrors(entries,unsortedX,graphY,tree->GetV4(),tree->GetV3());
    }
    else { graphNew = new TGraphErrors(entries,unsortedX,graphY,0,tree->GetV3()); }
  }
  else { graphNew = new TGraphErrors(entries,unsortedX,graphY,0,0); }
  // with "Set(...)", the x-axis is being sorted
  graphNew->GetXaxis()->Set(newNbins,newBins);

  // set the bins for the x-axis, apply shifting of points
  Char_t xName[50];
  for(Int_t i=0;i<count;i++){
    snprintf(xName,50,"%d",runNumber[i]);
    graphNew->GetXaxis()->SetBinLabel(i+1,xName);
    graphNew->GetX()[i]+=offset;
  }
  if (tree->GetVar(1)->IsInteger() && strlen(tree->GetHistogram()->GetXaxis()->GetBinLabel(1))>0){    
    for(Int_t i=0;i<count;i++){
      graphNew->GetXaxis()->SetBinLabel(i+1,tree->GetHistogram()->GetXaxis()->GetBinLabel(i+1));
    }
  }
  if (tree->GetVar(0)->IsInteger() &&  strlen(tree->GetHistogram()->GetXaxis()->GetBinLabel(1))>0 ){
    for(Int_t i=0;i<count;i++){
      graphNew->GetYaxis()->SetBinLabel(i+1,tree->GetHistogram()->GetYaxis()->GetBinLabel(i+1));
    }
  }

  graphNew->GetHistogram()->SetTitle("");
  graphNew->SetMarkerStyle(mstyle);
  graphNew->SetMarkerColor(mcolor);  graphNew->SetLineColor(mcolor);
  if (msize>0) { graphNew->SetMarkerSize(msize); graphNew->SetLineWidth(msize); }
  delete [] unsortedX;
  delete [] runNumber;
  delete [] index;
  delete [] newBins;
  // 
  TString chstring(expr);
  if (cut) chstring+=TString::Format(" ( %s )", cut);
  graphNew->SetTitle(chstring);

  THashList * metaData = (THashList*) tree->GetUserInfo()->FindObject("metaTable");
  if (metaData != NULL){
    chstring=expr;
    TObjArray *charray = chstring.Tokenize(":");
    graphNew->GetXaxis()->SetTitle(charray->At(1)->GetName());
    graphNew->GetYaxis()->SetTitle(charray->At(0)->GetName());    
    TNamed *nmdTitle0 = TStatToolkit::GetMetadata(tree,Form("%s.Title",charray->At(0)->GetName()));
    TNamed *nmdTitle1 = TStatToolkit::GetMetadata(tree,Form("%s.Title",charray->At(1)->GetName()));
    TNamed *nmdYAxis  = TStatToolkit::GetMetadata(tree,Form("%s.AxisTitle",charray->At(0)->GetName()));
    TNamed *nmdXAxis  = TStatToolkit::GetMetadata(tree,Form("%s.AxisTitle",charray->At(1)->GetName())); 
    //
    TString grTitle=charray->At(0)->GetName();
    if (nmdTitle0)  grTitle=nmdTitle0->GetTitle();
    if (nmdTitle1)  {
      grTitle+=":";
      grTitle+=nmdTitle1->GetTitle();
    }else{
      grTitle+=":";
      grTitle+=charray->At(1)->GetName();
    }
    if (cut)  grTitle+=TString::Format(" ( %s )", cut);
    graphNew->SetTitle(grTitle);
    if (nmdYAxis) {graphNew->GetYaxis()->SetTitle(nmdYAxis->GetTitle());}
    if (nmdXAxis) {graphNew->GetXaxis()->SetTitle(nmdXAxis->GetTitle());}            
    delete charray;
  }
  return graphNew;
}

/// Create MultiGraphs
///  \param tree       - tree containing data to be plot
///  \param groupname  - name of the output graph
///  \param expr       - string <variablearray>:<tagID>:<variablearrayError>
///                      <variablearray>="var0; ...; varN  
///                      <variablearrayError>="err0; ...;errN"
///  \param cut        - string "selection", resp. cut array
///                    - <variablearray>="defaultCut;cut0;cut1;...; cutN"
///  \param markers    - "marker0:...; markerN"
///  \param colors     - "color0:...; colorN"
///  \param drawSparse - swith use sparse drawing
///  \param comp       - swith: demanding that every variable in expr returned a plot OR accepting variables yielding no result
/// Example usage 
/// \code
/// Example usage of the TSTatToolkit::MakeMultGraph for T0 QA     :
///
///  legend=new TLegend(0.1,0.1,0.3,0.3,"T0 <Q> per sector");
///  legend->SetNColumns(2);
///  TMultiGraph * mGraph=TStatToolkit::MakeMultGraph(tree, "T0 <Q>","amplPMT1;amplPMT2;amplPMT3;amplPMT4;amplPMT5;amplPMT6:tagID:0.02;0.02;0.02;0.02;0.02;0.02","","21;22;25;26;27;28","1;2;4;3;856;616",kTRUE,0.9,10, legend);
///  mGraph->Draw();
///  legend->Draw();
/// \endcode
TMultiGraph * TStatToolkit::MakeMultGraph(TTree * tree, const char *groupName, const char* expr, const char * cut, const char * styles, const char *colors, Bool_t drawSparse, Float_t msize, Float_t sigmaRange, TLegend * legend, Bool_t comp){

  TMultiGraph *multiGraph=new TMultiGraph(groupName,groupName);
  TObjArray * exprVars=TString(expr).Tokenize(":");

  if (exprVars->GetEntries()<2) {
    ::Error("MakeMultGraph","NotValid expression %s",expr);
    delete exprVars;
    return 0;
  }
  TObjArray*exprVarArray = TString(exprVars->At(0)->GetName()).Tokenize(";");
  TObjArray*exprVarErrArray=(exprVars->GetEntries()>2)?  TString(exprVars->At(2)->GetName()).Tokenize(";"):0;
  TString cutString=(cut!=0)?cut:"1";
  if (cutString.Length()==0) cutString="1";
  TObjArray*exprCutArray= cutString.Tokenize(";");
  
  //determine marker style and line style
  const char* markerstyles;
  const char* linestyles=0;
  
  if(TString(styles).Contains(",")){    
  TString styls=TString(styles).ReplaceAll(" ","");
  TObjArray * stylarr=styls.Tokenize(","); 
  markerstyles=TString(stylarr->At(0)->GetName());
  linestyles=TString(stylarr->At(1)->GetName()); 
  }
  else markerstyles = styles;
  
  //determine marker style and line style
  const char* markercolors;
  const char* linecolors=0;
  
  if(TString(colors).Contains(",")){    
  TString cols=TString(colors).ReplaceAll(" ","");
  TObjArray * colarr=cols.Tokenize(","); 
  markercolors=TString(colarr->At(0)->GetName());
  linecolors=TString(colarr->At(1)->GetName()); 
  }
  else markercolors = colors;
//  ::Info("MakeMultGraph","Linestyle %d", atoi(linestyles));
    //Int markerStyle=AliDrawStyle::GetMarkerStyles(markers);
  Int_t notOK=(exprVarArray->GetEntries()<1 && exprCutArray->GetEntries()<2);
  if (exprVarErrArray) notOK+=2*(exprVarArray->GetEntriesFast()!=exprVarErrArray->GetEntriesFast());
  if (notOK>0){
    ::Error("MakeMultGraph","Not compatible arrays of variables:err variables or cuts - Problem %d", notOK);
    exprVarArray->Print();
    exprCutArray->Print();
    if (exprVarErrArray) exprVarErrArray->Print();
    delete  exprVars;
    return 0;
  }
  Int_t nCuts= TMath::Max(exprCutArray->GetEntries()-1,1);
  Int_t nExpr= exprVarArray->GetEntries();
  Int_t ngraphs = nCuts*nExpr;
  Double_t minValue=1;
  Double_t maxValue=-1;
  TVectorF vecMean(ngraphs);
  Bool_t flag = kFALSE;
  for (Int_t iGraph=0; iGraph<ngraphs; iGraph++){
    Int_t color=AliDrawStyle::GetMarkerColor(markercolors,iGraph);
    Int_t marker=AliDrawStyle::GetMarkerStyle(markerstyles, iGraph);
    Int_t iCut =iGraph%nCuts;
    Int_t iExpr=iGraph/nCuts;
    const char *expName=exprVarArray->At(iExpr)->GetName();
    const char *xName=exprVars->At(1)->GetName();
    const char *expErrName=(exprVarErrArray==NULL)? NULL:exprVarErrArray->At(iExpr)->GetName();
    TString cCut=exprCutArray->At(0)->GetName();
    if (nCuts>1){
      cCut+="&&";
      cCut+=exprCutArray->At(iCut+1)->GetName();
    }
    TGraph * gr = 0;
    if (drawSparse){
      if (exprVarErrArray==NULL){
        gr=TStatToolkit::MakeGraphSparse(tree, TString::Format("%s:%s",expName,xName).Data(),cCut.Data(), marker,color,msize);
      }else{
        gr=TStatToolkit::MakeGraphSparse(tree, TString::Format("%s:%s:%s",expName,xName,expErrName).Data(),cCut.Data(), marker,color,msize);
      }
    }else{
      if (exprVarErrArray==NULL){
        gr=TStatToolkit::MakeGraphErrors(tree, TString::Format("%s:%s",expName,xName).Data(),cCut.Data(), marker,color,msize);
      }else{
        gr=TStatToolkit::MakeGraphErrors(tree, TString::Format("%s:%s:%s",expName,xName,expErrName).Data(),cCut.Data(), marker,color,msize);
      }
    }
    
    if(linestyles!=0) gr->SetLineStyle(AliDrawStyle::GetLineStyle(linestyles,iGraph));
    if(linecolors!=0) gr->SetLineColor(AliDrawStyle::GetLineColor(linecolors,iGraph));
//      ::Info("MakeMultGraph","Linestyle2 %d", AliDrawStyle::GetLineStyle(linestyles,iGraph));
    
    if (gr) {
      if (marker<=0) 	{  // explicitly specify draw options - try to avoid bug in TMultiGraph draw in xaxis definition
        multiGraph->Add(gr);
      }else{
        if (iGraph==0){
          multiGraph->Add(gr,"ap");
        }else{
          multiGraph->Add(gr,"p");
        }
      }
    }
    Double_t meanT,rmsT=0;
    if (gr==NULL){
      if(comp){
        ::Error("MakeMultGraph","Not valid expression %s or cut %s - return",expName,cCut.Data());
        delete exprVarArray;
        delete exprVarErrArray;
        delete exprCutArray;
        return 0;
      }
      else{
        ::Error("MakeMultGraph","Not valid sub-expression %s or cut %s - continue",expName,cCut.Data());
        continue;
      }
    }
    flag=kTRUE;
    if (gr->GetN()>2){
      TStatToolkit::EvaluateUni(gr->GetN(),gr->GetY(), meanT,rmsT, TMath::Max(0.75*gr->GetN(),1.));
    }else{
      meanT=TMath::Median(gr->GetN(), gr->GetY());
      rmsT=TMath::RMS(gr->GetN(), gr->GetY());
    }
    if (maxValue<minValue){
      maxValue=meanT+sigmaRange*rmsT;
      minValue=meanT-sigmaRange*rmsT;
    }
    vecMean[iGraph]=meanT;
    if (minValue>meanT-sigmaRange*rmsT) minValue=meanT-sigmaRange*rmsT;
    if (maxValue<meanT+sigmaRange*rmsT) maxValue=meanT+sigmaRange*rmsT;
    TString prefix="";
    gr->SetName(expName);
    TNamed*named = TStatToolkit::GetMetadata(tree,TString::Format("%s.Title",expName).Data(),&prefix);
    if (named)  {
      gr->SetTitle(named->GetTitle());
    } else{
      gr->SetTitle(expName);
    }
    if (legend){
      TString legendName="";
      named = TStatToolkit::GetMetadata(tree,TString::Format("%s.Legend",expName).Data(),&prefix);
      if (named) {
        if (prefix.Length()>0){
          TString dummy=prefix+".Legend";
          if (TStatToolkit::GetMetadata(tree,dummy.Data())) {
            prefix=TStatToolkit::GetMetadata(tree,dummy.Data())->GetTitle();
          }
          legendName+=prefix.Data();
          legendName+=" ";
        }
        legendName+=named->GetTitle();
      }else{
        legendName=expName;
      }
      if (nCuts>1){
        legendName+=" ";
        named = TStatToolkit::GetMetadata(tree,TString::Format("%s.Legend",exprCutArray->At(iCut+1)->GetName()).Data(),&prefix);
        if (named) {
          legendName+=named->GetTitle();
        }else{
          legendName=exprCutArray->At(iCut+1)->GetName();
        }
      }
      legend->AddEntry(gr,legendName,"p");
    }


  }

  if(!flag){
    ::Error("Test::","Number of graphs 0 -return 0");
    delete exprVarArray;
    delete exprVarErrArray;
    //delete exprColors;
    //delete exprMarkers;
    return 0;
  }
  Double_t rmsGraphs = TMath::RMS(ngraphs,  vecMean.GetMatrixArray());
  minValue-=sigmaRange*rmsGraphs;
  maxValue+=sigmaRange*rmsGraphs;
  Double_t xmin=0,xmax=0;
  for (Int_t igr=0; igr<multiGraph->GetListOfGraphs()->GetSize(); igr++){
    TGraph* gr = (TGraph*)(multiGraph->GetListOfGraphs()->At(igr));
    gr->SetMinimum(minValue);
    gr->SetMaximum(maxValue);
    if (igr==0){
      xmin=gr->GetXaxis()->GetXmin();
      xmax=gr->GetXaxis()->GetXmax();
    }
    if (xmin>gr->GetXaxis()->GetXmin()) xmin=gr->GetXaxis()->GetXmin();
    if (xmax<gr->GetXaxis()->GetXmax()) xmax=gr->GetXaxis()->GetXmax();
  }
  // multiGraph->GetXaxis()->SetLimits(xmin,xmax); // BUG/FEATURE mutli graph axis not defined in this moment  (pointer==0)
  for (Int_t igr=0; igr<multiGraph->GetListOfGraphs()->GetSize(); igr++) {
    TGraph *gr = (TGraph *)(multiGraph->GetListOfGraphs()->At(igr));
    if (gr!=NULL) gr->GetXaxis()->SetLimits(xmin,xmax);
  }
  multiGraph->SetMinimum(minValue);
  multiGraph->SetMaximum(maxValue);
  delete exprVarArray;
  delete exprVarErrArray;
  delete exprCutArray;
  return multiGraph;
}


void   TStatToolkit::DrawMultiGraph(TMultiGraph *graph, Option_t * option){
  // 
  // TMultiGraph::Draw does not handle properly custom binning of the subcomponents
  // Using this function custom labels of the first graphs are used
  //
  static TPRegexp regAxis("^a");
  if (!graph) return;
  TList * grArray = graph->GetListOfGraphs();
  if (grArray==NULL) return;
  TGraph * gr0=(TGraph*)(grArray->At(0));
  if (gr0==NULL) return;
  gr0->Draw(option);   // defaults frame/axis defined by first graph
  Int_t ngr=grArray->GetEntries();
  TString option2(option);
  regAxis.Substitute(option2,"");
  for (Int_t igr=1; igr<ngr; igr++){
    grArray->At(igr)->Draw(option2.Data());
  }


}




//
// functions used for the trending
//


Int_t  TStatToolkit::MakeStatAlias(TTree * tree, const char * expr, const char * cut, const char * alias) 
{
  //
  // Add alias using statistical values of a given variable.
  // (by MI, Patrick Reichelt)
  //
  // tree - input tree
  // expr - variable expression
  // cut  - selection criteria
  // Output - return number of entries used to define variable
  // In addition mean, rms, median, and robust mean and rms (choosing fraction of data with smallest RMS)
  // 
  /* Example usage:
     1.) create the robust estimators for variable expr="QA.TPC.CPass1.meanTPCncl" and create a corresponding
     aliases with the prefix alias[0]="ncl", calculated using fraction alias[1]="0.90"

     TStatToolkit::MakeStatAlias(tree,"QA.TPC.CPass1.meanTPCncl","QA.TPC.CPass1.status>0","ncl:0.9");
     root [4] tree->GetListOfAliases().Print()
     OBJ: TNamed    ncl_Median      (130.964333+0)
     OBJ: TNamed    ncl_Mean        (122.120387+0)
     OBJ: TNamed    ncl_RMS         (33.509623+0)
     OBJ: TNamed    ncl_Mean90      (131.503862+0)
     OBJ: TNamed    ncl_RMS90       (3.738260+0)    
  */
  // 
  Int_t entries = tree->Draw(expr,cut,"goff");
  if (entries<=1){
    printf("Expression or cut not valid:\t%s\t%s\n", expr, cut);
    return 0;
  }
  //
  TObjArray* oaAlias = TString(alias).Tokenize(":");
  if (oaAlias->GetEntries()<2) {
    printf("Alias must have 2 arguments:\t%s\n", alias);
    return 0;
  }
  Float_t entryFraction = atof( oaAlias->At(1)->GetName() );
  //
  Double_t median = TMath::Median(entries,tree->GetV1());
  Double_t mean   = TMath::Mean(entries,tree->GetV1());
  Double_t rms    = TMath::RMS(entries,tree->GetV1());
  Double_t meanEF=0, rmsEF=0;
  TStatToolkit::EvaluateUni(entries, tree->GetV1(), meanEF, rmsEF, entries*entryFraction);
  //
  tree->SetAlias(Form("%s_Median",oaAlias->At(0)->GetName()), Form("(%f+0)",median));
  tree->SetAlias(Form("%s_Mean",oaAlias->At(0)->GetName()), Form("(%f+0)",mean));
  tree->SetAlias(Form("%s_RMS",oaAlias->At(0)->GetName()), Form("(%f+0)",rms));
  tree->SetAlias(Form("%s_Mean%d",oaAlias->At(0)->GetName(),Int_t(entryFraction*100)), Form("(%f+0)",meanEF));
  tree->SetAlias(Form("%s_RMS%d",oaAlias->At(0)->GetName(),Int_t(entryFraction*100)), Form("(%f+0)",rmsEF));
  delete oaAlias; 
  return entries;
}

Int_t  TStatToolkit::SetStatusAlias(TTree * tree, const char * expr, const char * cut, const char * alias) 
{
  //
  // Add alias to trending tree using statistical values of a given variable.
  // (by MI, Patrick Reichelt)
  //
  // format of expr :  varname (e.g. meanTPCncl)
  // format of cut  :  char like in TCut
  // format of alias:  alias:query:entryFraction(EF) (fraction of entries used for uniformity evaluation)
  //            e.g.:  varname_Out:(abs(varname-meanEF)>6.*rmsEF):0.8
  // available internal variables are: 'varname, Median, Mean, MeanEF, RMS, RMSEF'
  // in the alias, 'varname' will be replaced by its content, and 'EF' by the percentage (e.g. MeanEF -> Mean80)
  //
  /* Example usage:
     1.) Define robust mean (possible, but easier done with TStatToolkit::MakeStatAlias(...)) 
     TStatToolkit::SetStatusAlias(tree, "meanTPCnclF", "meanTPCnclF>0", "meanTPCnclF_MeanEF:MeanEF:0.80") ;
     root [10] tree->GetListOfAliases()->Print()
               Collection name='TList', class='TList', size=1
               OBJ: TNamed    meanTPCnclF_Mean80      0.899308
     2.) create alias outlyers  - 6 sigma cut
     TStatToolkit::SetStatusAlias(tree, "meanTPCnclF", "meanTPCnclF>0", "meanTPCnclF_Out:(abs(meanTPCnclF-MeanEF)>6.*RMSEF):0.8")
     meanTPCnclF_Out ==> (abs(meanTPCnclF-0.899308)>6.*0.016590)
     3.) the same functionality as in 2.)
     TStatToolkit::SetStatusAlias(tree, "meanTPCnclF", "meanTPCnclF>0", "varname_Out2:(abs(varname-MeanEF)>6.*RMSEF):0.8") 
     meanTPCnclF_Out2 ==> (abs(meanTPCnclF-0.899308)>6.*0.016590)
  */
  //
  Int_t entries = tree->Draw(expr,cut,"goff");
  if (entries<1){
    printf("Expression or cut not valid:\t%s\t%s\n", expr, cut);
    return 0;
  }
  //
  TObjArray* oaVar = TString(expr).Tokenize(":");
  char varname[50];
  snprintf(varname,50,"%s", oaVar->At(0)->GetName());
  Float_t entryFraction = 0.8;
  //
  TObjArray* oaAlias = TString(alias).Tokenize(":");
  if (oaAlias->GetEntries()<2) {
    printf("Alias must have at least 2 arguments:\t%s\n", alias);
    return 0;
  }
  else if (oaAlias->GetEntries()<3) {
    //printf("Using default entryFraction if needed:\t%f\n", entryFraction);
  }
  else entryFraction = atof( oaAlias->At(2)->GetName() );
  //
  Double_t median = TMath::Median(entries,tree->GetV1());
  Double_t mean   = TMath::Mean(entries,tree->GetV1());
  Double_t rms    = TMath::RMS(entries,tree->GetV1());
  Double_t meanEF=0, rmsEF=0;
  TStatToolkit::EvaluateUni(entries, tree->GetV1(), meanEF, rmsEF, entries*entryFraction);
  //
  TString sAlias( oaAlias->At(0)->GetName() );
  sAlias.ReplaceAll("varname",varname);
  sAlias.ReplaceAll("MeanEF", Form("Mean%1.0f",entryFraction*100) );
  sAlias.ReplaceAll("RMSEF",  Form("RMS%1.0f",entryFraction*100) );
  TString sQuery( oaAlias->At(1)->GetName() );
  sQuery.ReplaceAll("varname",varname);
  sQuery.ReplaceAll("MeanEF", Form("%f",meanEF) );
  sQuery.ReplaceAll("RMSEF",  Form("%f",rmsEF) ); //make sure to replace 'RMSEF' before 'RMS'...
  sQuery.ReplaceAll("Median", Form("%f",median) );
  sQuery.ReplaceAll("Mean",   Form("%f",mean) );
  sQuery.ReplaceAll("RMS",    Form("%f",rms) );
  printf("define alias:\t%s = %s\n", sAlias.Data(), sQuery.Data());
  //
  char query[200];
  char aname[200];
  snprintf(query,200,"%s", sQuery.Data());
  snprintf(aname,200,"%s", sAlias.Data());
  tree->SetAlias(aname, query);
  delete oaVar;
  delete oaAlias;
  return entries;
}


/// Build anchor aliases and check consistency of aliases strings
/// Input:
/// \param  tree       - input tree
/// \param  doCheck    - force check of the variables if valid formulas are used (slow down execution)
/// \param  verbose    - verbosity level 
/// \param sTrendVars  - semicolomn seprated list of tuples with 5 elements
///           tuple:      (variable, variableAnchor,  deltaWarning,deltaError, PhysAcc) 
///          <variable>_AnchorWarning:= |variable-variableAnchor|>deltaWarning
///          <variable>_AnchorError:= |variable-variableAnchor|>deltaError
///          <variable>_AnchorPhysAcceptable:= |variable-variableAnchor|>PhysAcc
///  Example configuration:
/*
  TString sTrendVars=";";   
  // Ncl
  sTrendVars+="QA.TPC.meanTPCncl,TPC.Anchor.meanTPCncl,5,10,5;";       // delta Ncl  warning 5 ,  error 10     (nominal ~ 100-140)
  sTrendVars+="QA.TPC.meanTPCnclF,TPC.Anchor.meanTPCnclF,0.02,0.05,0.05;"; // delta NclF  warning 2%,  error 5%    (nominal ~ 90%)
  // dcaR resolution
  sTrendVars+="QA.TPC.dcarAP0,TPC.Anchor.dcarAP0,0.02,0.05,0.02;";     // dcarAP0;  warning 0.02cm; error 0.05 cm  (nominal ~ 0.2 cm)

*/
void TStatToolkit::MakeAnchorAlias(TTree * tree, TString& sTrendVars, Int_t doCheck, Int_t verbose){
  const char* aType[4]={"Warning","Outlier","PhysAcc"};
  TObjArray *aTrendVars  = sTrendVars.Tokenize(";");
  Int_t entries= aTrendVars->GetEntries();
  const Int_t kMaxEntries=100;
  for (Int_t ientry=0; ientry<entries; ientry++){
    TString variables[kMaxEntries];
    Bool_t isOK=kTRUE;
    TObjArray *descriptor=TString(aTrendVars->At(ientry)->GetName()).Tokenize(",");
    // check if valid sysntax
    if (descriptor->GetEntries()!=5){
      ::Error("makeAnchorAlias"," Invalid status descriptor. %s has %d members instead of 5 (variable, deltaWarning,deltaError, PhysAcc) ",aTrendVars->At(ientry)->GetName(),descriptor->GetEntries());
      continue;
    }
    // check individual variables
    for (Int_t ivar=0; ivar<5; ivar++){
      variables[ivar]=descriptor->At(ivar)->GetName();
      if ((doCheck&1)>0){  // check input formulas in case specified
        TTreeFormula *form=new TTreeFormula("dummy",descriptor->At(ivar)->GetName(),tree);
        if (form->GetTree()==NULL){
          isOK=kFALSE;
          ::Error("makeAnchorAlias"," Invalid element %d,  %s in %s",ivar, descriptor->At(ivar)->GetName(),aTrendVars->At(ientry)->GetName());
          continue;
        }
      }
    }
    if (!isOK) continue;
    for (Int_t itype=0; itype<3; itype++) {
      TString aName = TString::Format("absDiff.%s_%s", variables[0].Data(), aType[itype]);
      TString aValue = "";
      if (itype < 2) {
        aValue = TString::Format("abs(%s-%s)>%s", variables[0].Data(), variables[1].Data(),
                                 variables[2 + itype].Data());  // Warning or Error
      } else {
        aValue = TString::Format("abs(%s-%s)<%s", variables[0].Data(), variables[1].Data(),
                                 variables[2 + itype].Data());  // Physics acceptable inside
      }
      tree->SetAlias(aName.Data(), aValue.Data());
      if ((doCheck & 2) > 0) {
        TTreeFormula *form = new TTreeFormula("dummy", aName.Data(), tree);
        if (form->GetTree() == NULL) {
          isOK = kFALSE;
          ::Error("makeAnchorAlias", "Alias not valid  \t%s\t%s", aName.Data(), aValue.Data());
        }
      }
      if (verbose > 0) {
        ::Info("makeAnchorAlias", "SetAlias\t%s\t%s", aName.Data(), aValue.Data());
      }
    }
    TString aName = TString::Format("absDiff.%s_", variables[0].Data());
    tree->SetAlias((aName+"WarningBand").Data(), (TString("1.*")+variables[2+0]).Data());
    tree->SetAlias((aName+"OutlierBand").Data(), (TString("1.*")+variables[2+1]).Data());
    tree->SetAlias((aName+"PhysAccBand").Data(), (TString("1.*")+variables[2+2]).Data());
	if (verbose > 0) {
    	::Info("makeAnchorAlias", "SetAlias \t%s\t%s", (aName+"WarningBand").Data(), variables[2+0].Data());
    }
    delete descriptor;
  }
  delete aTrendVars;
}

///  ==============================================================
///   Build combined aliases as a logical OR  of subexpressions
///          and check consistency  for combined status 
///   \param tree             - input tree
///   \param doCheck          - flag to check validity of aliases (time consuminng to evaluat formulas)
///   \param verbose          - specify verbosity (verbode>0 recomended)
///   \param sCombinedStatus  - coma separated list of expression
///                           - <combinedAlias, expr0, expr1, expr2, ....>  
///   Example: make combined status for the ncl and DCAResol
///    TString sCombinedStatus=";";
///    sCombinedStatus+="ncl,TPC.Anchor.meanTPCncl,TPC.Anchor.meanTPCnclF;";               // Status number of clusters and findable clusters
///    sCombinedStatus+="dcarResol,TPC.Anchor.dcarAP0,TPC.Anchor.dcarAP1,TPC.Anchor.dcarCP0,TPC.Anchor.dcarCP1;";  // Status: DCA resolution
///  ==============================================================
void TStatToolkit::MakeCombinedAlias(TTree * tree, TString& sCombinedStatus, Bool_t doCheck, Int_t verbose ){
  const char* aType[3]={"_Warning","_Outlier","_PhysAcc"};
  TObjArray *aTrendStatus  = sCombinedStatus.Tokenize(";");
  Int_t entries= aTrendStatus->GetEntries();
  const Int_t kMaxEntries=100;
  for (Int_t ientry=0; ientry<entries; ientry++){
    TString variables[kMaxEntries];
    Bool_t isOK=kTRUE;
    TObjArray *descriptor=TString(aTrendStatus->At(ientry)->GetName()).Tokenize(",");
    // check if valid sysntax
    Int_t nElems=descriptor->GetEntries();
    if (nElems<2){
      ::Error("makeCombinedAlias"," Invalid combined status descriptor. Too few entries %d in status %s", nElems, aTrendStatus->At(ientry)->GetName());
      continue;
    }
    // check individual variables
    TString aliasName[3], aliasContent[3];
    for (Int_t ivar=0; ivar<nElems; ivar++){
      if ((doCheck&1)>0 &&ivar>0) {
	TTreeFormula *form=new TTreeFormula("dummy",descriptor->At(ivar)->GetName(),tree);
	isOK=form->GetTree()!=NULL;
      }
      if (!isOK){
	isOK=kFALSE;
	::Error("makeCombinedAlias"," Invalid element %d,  %s in %s ",ivar, descriptor->At(ivar)->GetName(),aTrendStatus->At(ientry)->GetName());
	break;
      }else{
	variables[ivar]=descriptor->At(ivar)->GetName();
	for (Int_t iType=0; iType<3; iType++){ //type loop	  
	  if (ivar==0) {
	    aliasName[iType]=descriptor->At(ivar)->GetName();
	    aliasName[iType]+=aType[iType];
	  }
	  else{
	    if (ivar==1) aliasContent[iType]="(";
	    aliasContent[iType]+=descriptor->At(ivar)->GetName();
	    aliasContent[iType]+=aType[iType];
	    if (ivar<nElems-1)  aliasContent[iType]+="||";
	    if (ivar==nElems-1) aliasContent[iType]+=")";
	  }
	}
      }
    }
    if (isOK){
      for (Int_t iType=0; iType<3; iType++){
	tree->SetAlias(aliasName[iType].Data(), aliasContent[iType].Data());
	if (verbose>0){
	  ::Info("makeCombinedAlias","SetAlias\t%s\t%s", aliasName[iType].Data(),aliasContent[iType].Data());
	}   
      }
    }
    if (!isOK) continue;
    delete descriptor;
  }
  delete aTrendStatus;
}  


/// Compute a trending multigraph that shows for which runs a variable has outliers.
/// format of expr :  varname:xaxis (e.g. meanTPCncl:run, but 'varname' can be any string that you need for seach-and-replace)
/// format of cut  :  char like in TCut
/// format of alias:  (1):(statisticOK):(varname_Warning):(varname_Out)[:(varname_PhysAcc):(varname_Extra)]
///
/// function MakeGraphSparse() is called for each alias argument, which will be used as tree expression.
/// each alias argument is supposed to be a Boolean statement which can be evaluated as tree expression.
/// the order of these criteria should be kept, as the marker styles and colors are chosen to be meaningful this way!
/// 'statisticOK' could e.g. be an alias for '(meanTPCncl>0)'.
/// if you dont need e.g. a 'warning' condition, then just replace it by (0).
/// in the alias, 'varname' will be replaced by its content (e.g. varname_Out -> meanTPCncl_Out)
/// note: the aliases 'varname_Out' etc have to be defined by function TStatToolkit::SetStatusAlias(...)
/// counter igr is used to shift the multigraph in y when filling a TObjArray.
///  \param tree       - tree containing data to be plot
///  \param expr       - string <variablearray>:<tagID>:<variablearrayError>
///                      <variablearray>="var0; ...; varN  
///                      <variablearrayError>="err0; ...;errN"
///  \param alias      - Boolean statement which can be evaluated as tree expression
///  \param igr        - counter used to shift the multigraph in y when filling a TObjArray
/// Example usage 
/// \code
/// To create the Status Bar, the following is done in principle.
/// ( example current usage in $ALICE_ROOT/PWGPP/TPC/macros/drawPerformanceTPCQAMatchTrends.C and ./qaConfig.C. )
///
/// TStatToolkit::SetStatusAlias(tree, "meanTPCncl",    "", "varname_Out:(abs(varname-MeanEF)>6.*RMSEF):0.8");
/// TStatToolkit::SetStatusAlias(tree, "tpcItsMatchA",  "", "varname_Out:(abs(varname-MeanEF)>6.*RMSEF):0.8");
/// TStatToolkit::SetStatusAlias(tree, "meanTPCncl",    "", "varname_Warning:(abs(varname-MeanEF)>3.*RMSEF):0.8");
/// TStatToolkit::SetStatusAlias(tree, "tpcItsMatchA",  "", "varname_Warning:(abs(varname-MeanEF)>3.*RMSEF):0.8");
/// TObjArray* oaMultGr = new TObjArray(); int igr=0;
/// oaMultGr->Add( TStatToolkit::MakeStatusMultGr(tree, "tpcItsMatchA:run",  "", "(1):(meanTPCncl>0):(varname_Warning):(varname_Outlier):", igr) ); igr++;
/// oaMultGr->Add( TStatToolkit::MakeStatusMultGr(tree, "meanTPCncl:run",    "", "(1):(meanTPCncl>0):(varname_Warning):(varname_Outlier):", igr) ); igr++;
/// TCanvas *c1 = new TCanvas("c1","c1");
/// TStatToolkit::AddStatusPad(c1, 0.30, 0.40);
/// TStatToolkit::DrawStatusGraphs(oaMultGr);
/// \endcode  

TMultiGraph*  TStatToolkit::MakeStatusMultGr(TTree * tree, const char * expr, const char * cut, const char * alias, Int_t igr) 
{

  TObjArray* oaVar = TString(expr).Tokenize(":");
  if (oaVar->GetEntries()<2) {
    printf("Expression has to be of type 'varname:xaxis':\t%s\n", expr);
    return 0;
  }
  char varname[50];
  char var_x[50];
  snprintf(varname,50,"%s", oaVar->At(0)->GetName());
  snprintf(var_x  ,50,"%s", oaVar->At(1)->GetName());
  //
  TString sAlias(alias);
  sAlias.ReplaceAll("varname",varname);
  TObjArray* oaAlias = TString(sAlias.Data()).Tokenize(":");
  if (oaAlias->GetEntries()<2) {
    printf("Alias must have 2-6 arguments:\t%s\n", alias);
    return 0;
  }
  char query[200];
  TMultiGraph* multGr = new TMultiGraph();
  Int_t marArr[6]      = {24+igr%2, 20+igr%2, 20+igr%2, 20+igr%2, 20+igr%2, 20+igr%2};
  Int_t colArr[6]      = {kBlack, kBlack, kOrange, kRed, kGreen+1, kBlue};
  Double_t sizeArr[6]  = {1.4, 1.1, 1.5, 1.1, 1.4, 0.8};
  Double_t shiftArr[6] = {0., 0., 0.25, 0.25, -0.25, -0.25};
  const Int_t ngr = oaAlias->GetEntriesFast();
  for (Int_t i=0; i<ngr; i++){
    snprintf(query,200, "%f*(%s-0.5):%s", 1.+igr, oaAlias->At(i)->GetName(), var_x);
    TGraphErrors * gr = (TGraphErrors*) TStatToolkit::MakeGraphSparse(tree,query,cut,marArr[i],colArr[i],sizeArr[i],shiftArr[i]);
    if (gr) {
      multGr->Add(gr);
      gr->SetName(oaAlias->At(i)->GetName());
      gr->SetTitle(oaAlias->At(i)->GetName());
    }
    else{
        ::Error("TStatToolkit::MakeStatusMultGr","MakeGraphSparse() returned with error when called with the following query: %s",query);
        return 0;
    }
  }
  //
  multGr->SetName(varname);
  multGr->SetTitle(varname); // used for y-axis labels of status bar, can be modified by calling function.
  delete oaVar;
  delete oaAlias;
  return multGr;
}


void  TStatToolkit::AddStatusPad(TCanvas* c1, Float_t padratio, Float_t bottommargin)
{
  //
  // add pad to bottom of canvas for Status graphs (by Patrick Reichelt)
  // call function "DrawStatusGraphs(...)" afterwards
  //
  TCanvas* c1_clone = (TCanvas*) c1->Clone("c1_clone");
  c1->Clear();
  // produce new pads
  c1->cd();
  TPad* pad1 = new TPad("pad1", "pad1", 0., padratio, 1., 1.); 
  pad1->Draw();
  pad1->SetNumber(1); // so it can be called via "c1->cd(1);"
  c1->cd();
  TPad* pad2 = new TPad("pad2", "pad2", 0., 0., 1., padratio);
  pad2->Draw();
  pad2->SetNumber(2);
  // draw original canvas into first pad
  c1->cd(1);
  c1_clone->DrawClonePad();
  pad1->SetBottomMargin(0.001);
  pad1->SetRightMargin(0.01);
  // set up second pad
  c1->cd(2);
  pad2->SetGrid(3);
  pad2->SetTopMargin(0);
  pad2->SetBottomMargin(bottommargin); // for the long x-axis labels (runnumbers)
  pad2->SetRightMargin(0.01);
}


void  TStatToolkit::DrawStatusGraphs(TObjArray* oaMultGr)
{
  //
  // draw Status graphs into active pad of canvas (by MI, Patrick Reichelt)
  // ...into bottom pad, if called after "AddStatusPad(...)"
  //
  const Int_t nvars = oaMultGr->GetEntriesFast();
  TGraph* grAxis = (TGraph*) ((TMultiGraph*) oaMultGr->At(0))->GetListOfGraphs()->At(0);
  grAxis->SetMaximum(0.5*nvars+0.5);
  grAxis->SetMinimum(0);
  grAxis->GetYaxis()->SetLabelSize(0);
  grAxis->GetYaxis()->SetTitle("");
  grAxis->SetTitle("");
  Int_t entries = grAxis->GetN();
  grAxis->GetXaxis()->SetLabelSize(5.7*TMath::Min(TMath::Max(5./entries,0.01),0.03));
  grAxis->GetXaxis()->LabelsOption("v");
  grAxis->Draw("ap");
  //
  // draw multigraphs & names of status variables on the y axis
  for (Int_t i=0; i<nvars; i++){
    ((TMultiGraph*) oaMultGr->At(i))->Draw("p");
    TLatex* ylabel = new TLatex(-0.1, 0.5*i+0.5, ((TMultiGraph*) oaMultGr->At(i))->GetTitle());
    ylabel->SetTextAlign(32); //hor:right & vert:centered
    ylabel->SetTextSize(0.025/gPad->GetHNDC());
    ylabel->Draw();
  }
}


TTree*  TStatToolkit::WriteStatusToTree(TObject* oStatusGr) 
{
  //
  // Create Tree with Integers for each status variable flag (warning, outlier, physacc).
  // (by Patrick Reichelt)
  //
  // input: either a TMultiGraph with status of single variable, which 
  //        was computed by TStatToolkit::MakeStatusMultGr(),
  //        or a TObjArray which contains up to 10 of such variables.
  //        example: TTree* statusTree = WriteStatusToTree( TStatToolkit::MakeStatusMultGr(tree, "tpcItsMatch:run",  "", sCriteria.Data(), 0) );
  //        or     : TTree* statusTree = TStatToolkit::WriteStatusToTree(oaMultGr);
  // 
  // output tree: 1=flag is true, 0=flag is false, -1=flag was not computed.
  // To be rewritten to the pcstream
  
  TObjArray* oaMultGr = NULL;
  Bool_t needDeletion=kFALSE;
  if (oStatusGr->IsA() == TObjArray::Class()) {
    oaMultGr = (TObjArray*) oStatusGr;
  }
  else if (oStatusGr->IsA() == TMultiGraph::Class()) {
    oaMultGr = new TObjArray(); needDeletion=kTRUE;
    oaMultGr->Add((TMultiGraph*) oStatusGr);
  }
  else {
    Printf("WriteStatusToTree(): Error! 'oStatusGr' must be a TMultiGraph or a TObjArray of them!");
    return 0;
  }
  // variables for output tree
  const int nvarsMax=10;
  const int ncritMax=5;
  Int_t    currentRun;
  Int_t    treevars[nvarsMax*ncritMax];
  TString  varnames[nvarsMax*ncritMax];
  for (int i=0; i<nvarsMax*ncritMax; i++) treevars[i]=-1;
  
  Printf("WriteStatusToTree(): writing following variables to TTree (maybe only subset of listed criteria filled)");
  for (Int_t vari=0; vari<nvarsMax; vari++) 
  {
    if (vari < oaMultGr->GetEntriesFast()) {
      varnames[vari*ncritMax+0] = Form("%s_statisticOK", ((TMultiGraph*) oaMultGr->At(vari))->GetName());
      varnames[vari*ncritMax+1] = Form("%s_Warning",     ((TMultiGraph*) oaMultGr->At(vari))->GetName());
      varnames[vari*ncritMax+2] = Form("%s_Outlier",     ((TMultiGraph*) oaMultGr->At(vari))->GetName());
      varnames[vari*ncritMax+3] = Form("%s_PhysAcc",     ((TMultiGraph*) oaMultGr->At(vari))->GetName());
      varnames[vari*ncritMax+4] = Form("%s_Extra",       ((TMultiGraph*) oaMultGr->At(vari))->GetName());
    }
    else {
      varnames[vari*ncritMax+0] = Form("dummy");
      varnames[vari*ncritMax+1] = Form("dummy");
      varnames[vari*ncritMax+2] = Form("dummy");
      varnames[vari*ncritMax+3] = Form("dummy");
      varnames[vari*ncritMax+4] = Form("dummy");
    }
    cout << "  " << varnames[vari*ncritMax+0].Data() << " " << varnames[vari*ncritMax+1].Data() << " " << varnames[vari*ncritMax+2].Data() << " " << varnames[vari*ncritMax+3].Data() << " " << varnames[vari*ncritMax+4].Data() << endl;
  }
  
  TTree* statusTree = new TTree("statusTree","statusTree");
  statusTree->Branch("run",                &currentRun  );
  statusTree->Branch(varnames[ 0].Data(),  &treevars[ 0]);
  statusTree->Branch(varnames[ 1].Data(),  &treevars[ 1]);
  statusTree->Branch(varnames[ 2].Data(),  &treevars[ 2]);
  statusTree->Branch(varnames[ 3].Data(),  &treevars[ 3]);
  statusTree->Branch(varnames[ 4].Data(),  &treevars[ 4]);
  statusTree->Branch(varnames[ 5].Data(),  &treevars[ 5]);
  statusTree->Branch(varnames[ 6].Data(),  &treevars[ 6]);
  statusTree->Branch(varnames[ 7].Data(),  &treevars[ 7]);
  statusTree->Branch(varnames[ 8].Data(),  &treevars[ 8]);
  statusTree->Branch(varnames[ 9].Data(),  &treevars[ 9]);
  statusTree->Branch(varnames[10].Data(),  &treevars[10]);
  statusTree->Branch(varnames[11].Data(),  &treevars[11]);
  statusTree->Branch(varnames[12].Data(),  &treevars[12]);
  statusTree->Branch(varnames[13].Data(),  &treevars[13]);
  statusTree->Branch(varnames[14].Data(),  &treevars[14]);
  statusTree->Branch(varnames[15].Data(),  &treevars[15]);
  statusTree->Branch(varnames[16].Data(),  &treevars[16]);
  statusTree->Branch(varnames[17].Data(),  &treevars[17]);
  statusTree->Branch(varnames[18].Data(),  &treevars[18]);
  statusTree->Branch(varnames[19].Data(),  &treevars[19]);
  statusTree->Branch(varnames[20].Data(),  &treevars[20]);
  statusTree->Branch(varnames[21].Data(),  &treevars[21]);
  statusTree->Branch(varnames[22].Data(),  &treevars[22]);
  statusTree->Branch(varnames[23].Data(),  &treevars[23]);
  statusTree->Branch(varnames[24].Data(),  &treevars[24]);
  statusTree->Branch(varnames[25].Data(),  &treevars[25]);
  statusTree->Branch(varnames[26].Data(),  &treevars[26]);
  statusTree->Branch(varnames[27].Data(),  &treevars[27]);
  statusTree->Branch(varnames[28].Data(),  &treevars[28]);
  statusTree->Branch(varnames[29].Data(),  &treevars[29]);
  statusTree->Branch(varnames[30].Data(),  &treevars[30]);
  statusTree->Branch(varnames[31].Data(),  &treevars[31]);
  statusTree->Branch(varnames[32].Data(),  &treevars[32]);
  statusTree->Branch(varnames[33].Data(),  &treevars[33]);
  statusTree->Branch(varnames[34].Data(),  &treevars[34]);
  statusTree->Branch(varnames[35].Data(),  &treevars[35]);
  statusTree->Branch(varnames[36].Data(),  &treevars[36]);
  statusTree->Branch(varnames[37].Data(),  &treevars[37]);
  statusTree->Branch(varnames[38].Data(),  &treevars[38]);
  statusTree->Branch(varnames[39].Data(),  &treevars[39]);
  statusTree->Branch(varnames[40].Data(),  &treevars[40]);
  statusTree->Branch(varnames[41].Data(),  &treevars[41]);
  statusTree->Branch(varnames[42].Data(),  &treevars[42]);
  statusTree->Branch(varnames[43].Data(),  &treevars[43]);
  statusTree->Branch(varnames[44].Data(),  &treevars[44]);
  statusTree->Branch(varnames[45].Data(),  &treevars[45]);
  statusTree->Branch(varnames[46].Data(),  &treevars[46]);
  statusTree->Branch(varnames[47].Data(),  &treevars[47]);
  statusTree->Branch(varnames[48].Data(),  &treevars[48]);
  statusTree->Branch(varnames[49].Data(),  &treevars[49]);
  
  // run loop
  Double_t graphX; // x-position of marker (0.5, 1.5, ...)
  Double_t graphY; // if >0 -> warning/outlier/physacc! if =-0.5 -> no warning/outlier/physacc
  TList* arrRuns = (TList*) ((TGraph*) ((TMultiGraph*) oaMultGr->At(0))->GetListOfGraphs()->At(0))->GetXaxis()->GetLabels();
  //'TAxis->GetLabels()' returns THashList of TObjString, but using THashList gives compilation error "... incomplete type 'struct THashList' "
  for (Int_t runi=0; runi<arrRuns->GetSize(); runi++) 
  {
    currentRun = atoi( arrRuns->At(runi)->GetName() );
    //Printf(" runi=%2i, name: %s \t run number: %i", runi, arrRuns->At(runi)->GetName(), currentRun);
    
    // status variable loop
    for (Int_t vari=0; vari<oaMultGr->GetEntriesFast(); vari++) 
    {
      TMultiGraph* multGr = (TMultiGraph*) oaMultGr->At(vari);
      
      // criteria loop
      // the order is given by TStatToolkit::MakeStatusMultGr().
      // criterion #1 is 'statisticOK' and mandatory, the rest is optional. (#0 is always True, thus skipped)
      for (Int_t criti=1; criti<multGr->GetListOfGraphs()->GetEntries(); criti++) 
      {
        TGraph* grCriterion = (TGraph*) multGr->GetListOfGraphs()->At(criti);
        graphX = -1, graphY = -1;
        grCriterion->GetPoint(runi, graphX, graphY);
        treevars[(vari)*ncritMax+(criti-1)] = (graphY>0)?1:0;
      }
    }
    statusTree->Fill();
  }
  
  if (needDeletion) delete oaMultGr;
  
  return statusTree;
}


void   TStatToolkit::MakeSummaryTree(TTree* treeIn, TTreeSRedirector *pcstream, TObjString & sumID, TCut &selection){
  //
  // Make a  summary tree for the input tree 
  // For the moment statistic works only for the primitive branches (Float/Double/Int)
  // Extension recursive version planned for graphs a and histograms
  //
  // Following statistics are exctracted:
  //   - Standard: mean, meadian, rms
  //   - LTM robust statistic: mean60, rms60, mean90, rms90
  // Parameters:
  //    treeIn    - input tree 
  //    pctream   - Output redirector
  //    sumID     - ID as will be used in output tree
  //    selection - selection criteria define the set of entries used to evaluat statistic 
  //
  // Curently only predefined statistic used to fill summary information
  // Future plans an option user defined statistic descriptor instead of the defualt (if exist)
  // 
  //      e.g 
  //          default.Branches=median:mean90:rms90:mean60:rms60
  //          interactionRate.Branches   mean90:median:rms90:mean95:rms95:mean60:rms60
  //
  TObjArray * brArray = treeIn->GetListOfBranches();
  Int_t tEntries= treeIn->GetEntries();
  Int_t nBranches=brArray->GetEntries();
  TString treeName = treeIn->GetName();

  (*pcstream)<<treeName.Data()<<"entries="<<tEntries;
  (*pcstream)<<treeName.Data()<<"ID.="<<&sumID;
  
  TMatrixD valBranch(nBranches,7);
  for (Int_t iBr=0; iBr<nBranches; iBr++){    
    TString brName= brArray->At(iBr)->GetName();
    Int_t entries=treeIn->Draw(TString::Format("%s>>dummy(10,0,1)",brArray->At(iBr)->GetName()).Data(),selection,"goff");
    if (entries==0) continue;
    Double_t median, mean, rms, mean60,rms60, mean90, rms90;
    mean  = TMath::Mean(entries,treeIn->GetV1());
    median= TMath::Median(entries,treeIn->GetV1());
    rms   = TMath::RMS(entries,treeIn->GetV1());
    TStatToolkit::EvaluateUni(entries, treeIn->GetV1(), mean60,rms60,TMath::Min(TMath::Max(2., 0.60*entries),Double_t(entries)));
    TStatToolkit::EvaluateUni(entries, treeIn->GetV1(), mean90,rms90,TMath::Min(TMath::Max(2., 0.90*entries),Double_t(entries)));
    valBranch(iBr,0)=mean; 
    valBranch(iBr,1)=median; 
    valBranch(iBr,2)=rms; 
    valBranch(iBr,3)=mean60; 
    valBranch(iBr,4)=rms60; 
    valBranch(iBr,5)=mean90; 
    valBranch(iBr,6)=rms90; 
    (*pcstream)<<treeName.Data()<<
      brName+"="<<valBranch(iBr,1)<<           // use as an default median estimator
      brName+"_Mean="<<valBranch(iBr,0)<< 
      brName+"_Median="<<valBranch(iBr,1)<<
      brName+"_RMS="<<valBranch(iBr,2)<<
      brName+"_Mean60="<<valBranch(iBr,3)<<
      brName+"_RMS60="<<valBranch(iBr,4)<<
      brName+"_Mean90="<<valBranch(iBr,5)<<
      brName+"_RMS90="<<valBranch(iBr,6);  
  }
  (*pcstream)<<treeName.Data()<<"\n";
}



TMultiGraph*  TStatToolkit::MakeStatusLines(TTree * tree, const char * expr, const char * cut, const char * alias) 
{
  //
  // Create status lines for trending using MakeGraphSparse(), very similar to MakeStatusMultGr().
  // (by Patrick Reichelt)
  //
  // format of expr :  varname:xaxis (e.g. meanTPCncl:run, but 'varname' can be any string that you need for seach-and-replace)
  // format of cut  :  char like in TCut
  // format of alias:  varname_OutlierMin:varname_OutlierMax:varname_WarningMin:varname_WarningMax:varname_PhysAccMin:varname_PhysAccMax:varname_RobustMean
  //
  TObjArray* oaVar = TString(expr).Tokenize(":");
  if (oaVar->GetEntries()<2) {
    printf("Expression has to be of type 'varname:xaxis':\t%s\n", expr);
    return 0;
  }
  char varname[50];
  char var_x[50];
  snprintf(varname,50,"%s", oaVar->At(0)->GetName());
  snprintf(var_x  ,50,"%s", oaVar->At(1)->GetName());
  //
  TString sAlias(alias);
  if (sAlias.IsNull()) { // alias for default usage set here:
    sAlias = "varname_OutlierMin:varname_OutlierMax:varname_WarningMin:varname_WarningMax:varname_PhysAccMin:varname_PhysAccMax:varname_RobustMean";
  }
  sAlias.ReplaceAll("varname",varname);
  TObjArray* oaAlias = TString(sAlias.Data()).Tokenize(":");
  if (oaAlias->GetEntries()<2) {
    printf("Alias must have 2-7 arguments:\t%s\n", alias);
    return 0;
  }
  char query[200];
  TMultiGraph* multGr = new TMultiGraph();
  Int_t colArr[7] = {kRed, kRed, kOrange, kOrange, kGreen+1, kGreen+1, kGray+2};
  const Int_t ngr = oaAlias->GetEntriesFast();
  for (Int_t i=0; i<ngr; i++){
    snprintf(query,200, "%s:%s", oaAlias->At(i)->GetName(), var_x);
    multGr->Add( (TGraphErrors*) TStatToolkit::MakeGraphSparse(tree,query,cut,29,colArr[i],1.5) );
  }
  //
  multGr->SetName(varname);
  multGr->SetTitle(varname);
  delete oaVar;
  delete oaAlias;
  return multGr;
}


TH1* TStatToolkit::DrawHistogram(TTree * tree, const char* drawCommand, const char* cuts, const char* histoname, const char* histotitle, Int_t nsigma, Float_t fraction, TObjArray *description )
{
  //
  // Draw histogram from TTree with robust range
  // Only for 1D so far!
  // 
  // Parameters:
  // - histoname:  name of histogram
  // - histotitle: title of histgram
  // - fraction:   fraction of data to define the robust mean
  // - nsigma:     nsigma value for range
  //
  // To add:
  //    automatic ranges - separatelly for X, Y and Z nbins  - as string
  //    names for the variables
  //    option, entries, first entry  like in tree draw
  //
   TString drawStr(drawCommand);
   TString cutStr(cuts);
   Int_t dim = 1;

   if(!tree) {
     ::Error("TStatToolkit::DrawHistogram","Tree pointer is NULL!");
     return 0;
   }

   // get entries
   Int_t entries = tree->Draw(drawStr.Data(), cutStr.Data(), "goff");
   if (entries == -1) {
     ::Error("TStatToolkit::DrawHistogram","Tree draw returns -!");
     return 0;
   }
   TObjArray *charray = drawStr.Tokenize(":");

   // get dimension
   if(tree->GetV1()) dim = 1;
   if(tree->GetV2()) dim = 2;
   if(tree->GetV3()) dim = 3;
   if(dim > 2){
     cerr<<"TTree has more than 2 dimensions (not yet supported)"<<endl;
     return 0;
   }

   // draw robust
   // Get estimators
   Double_t mean1=0, rms1=0, min1=0, max1=0;
   Double_t mean2=0, rms2=0, min2=0, max2=0;
   Double_t mean3=0, rms3=0, min3=0, max3=0;
   
   TStatToolkit::GetMinMaxMean( tree->GetV1(),entries, min1,max1, mean1);  
   TStatToolkit::EvaluateUni(entries, tree->GetV1(),mean1,rms1, fraction*entries);
   if(dim>1){
     TStatToolkit::GetMinMaxMean( tree->GetV2(),entries, min2,max2, mean2);  
     TStatToolkit::EvaluateUni(entries, tree->GetV1(),mean2,rms2, fraction*entries);
   }
   if(dim>2){
     TStatToolkit::GetMinMaxMean( tree->GetV3(),entries, min3,max3, mean3);  
     TStatToolkit::EvaluateUni(entries, tree->GetV3(),mean3,rms3, fraction*entries);
   }

   TH1* hOut=NULL;
   if(dim==1){
     hOut = new TH1F(histoname, histotitle, 200, mean1-nsigma*rms1, mean1+nsigma*rms1);
     for (Int_t i=0; i<entries; i++) hOut->Fill(tree->GetV1()[i]);
     hOut->GetXaxis()->SetTitle(tree->GetHistogram()->GetXaxis()->GetTitle());
   }
   else if(dim==2){
     hOut = new TH2F(histoname, histotitle, 200, min2, max2,200, mean1-nsigma*rms1, mean1+nsigma*rms1);
     for (Int_t i=0; i<entries; i++) hOut->Fill(tree->GetV2()[i],tree->GetV1()[i]);
     hOut->GetXaxis()->SetTitle(tree->GetHistogram()->GetXaxis()->GetTitle());
     hOut->GetYaxis()->SetTitle(tree->GetHistogram()->GetYaxis()->GetTitle());
   }
   THashList * metaData = (THashList*) tree->GetUserInfo()->FindObject("metaTable");
   
   if (metaData != NULL){
    TNamed *nmdTitle0 = TStatToolkit::GetMetadata(tree,Form("%s.Title",charray->At(0)->GetName()));
    TNamed *nmdXAxis  = TStatToolkit::GetMetadata(tree,Form("%s.AxisTitle",charray->At(1)->GetName())); 
    TNamed *nmdTitle1 = TStatToolkit::GetMetadata(tree,Form("%s.Title",charray->At(1)->GetName()));
    TNamed *nmdYAxis  = TStatToolkit::GetMetadata(tree,Form("%s.AxisTitle",charray->At(0)->GetName()));
    //
    TString hisTitle=charray->At(0)->GetName();
    if (nmdTitle0)  hisTitle=nmdTitle0->GetTitle();
    if (nmdTitle1)  {
      hisTitle+=":";
      hisTitle+=nmdTitle1->GetTitle();
    }else{
      hisTitle+=":";
      hisTitle+=charray->At(1)->GetName();
    }
    if (nmdYAxis) {hOut->GetYaxis()->SetTitle(nmdYAxis->GetTitle());}
    if (nmdXAxis) {hOut->GetXaxis()->SetTitle(nmdXAxis->GetTitle());}            
    hOut->SetTitle(hisTitle);
  }
  delete charray;
  // if (option) hOut->Draw(option);
  return hOut;
}

void TStatToolkit::CheckTreeAliases(TTree * tree, Int_t ncheck){
  //
  // Check consistency of tree aliases
  //
  Int_t nCheck=100;
  TList * aliases = (TList*)tree->GetListOfAliases();
  Int_t entries = aliases->GetEntries();
  for (Int_t i=0; i<entries; i++){
    TObject * object= aliases->At(i);
    if (!object) continue;
    Int_t ndraw=tree->Draw(aliases->At(i)->GetName(),"1","goff",nCheck);
    if (ndraw==0){
      ::Error("Alias:\tProblem","%s",aliases->At(i)->GetName());
    }else{
      ::Info("Alias:\tOK","%s",aliases->At(i)->GetName());
    }
  }
}




Double_t TStatToolkit::GetDefaultStat(TTree * tree, const char * var, const char * selection, TStatType statType){
  //
  //
  //
  Int_t entries = tree->Draw(var,selection,"goff");
  if (entries==0) return 0;
  switch(statType){    
  case kEntries:    
    return entries;
  case kSum:    
    return entries*TMath::Mean(entries, tree->GetV1());
  case kMean:    
    return TMath::Mean(entries, tree->GetV1());
  case kRMS:     
    return TMath::RMS(entries, tree->GetV1());
  case kMedian:  
    return TMath::Median(entries, tree->GetV1());    
  default:;
  }
  return 0;
}

//_____________________________________________________________________________
void TStatToolkit::CombineArray(TTree *tree, TVectorD &values)
{
  /// Collect all variables from the last draw in one array.
  ///
  /// It is assumed that the Draw function of the TTree was called before
  /// if e.g. Draw("v1:v2:v3") had been called, then values will contain
  /// the concatenated array of the values from v1,v2 and v3.
  /// E.g. if the v1[0..n], v2[0..n], v3[0..n] then
  /// values[0..3n] = [v1, v2, v3]
  /// \param[in]  tree   input tree
  /// \param[out] values array in which to summarise all 'drawn' values
  const Int_t numberOfDimensions = tree->GetPlayer()->GetDimension();
  if (numberOfDimensions==1) {
    values.Use(tree->GetSelectedRows(), tree->GetVal(0));
    return;
  }

  const Int_t numberOfSelectedRows = tree->GetSelectedRows();
  values.ResizeTo(numberOfDimensions * numberOfSelectedRows);

  Int_t nfill=0;
  for (Int_t idim=0; idim<numberOfDimensions; ++idim) {
    const Double_t *arr = tree->GetVal(idim);
    if (!arr) continue;

    for (Int_t ival=0; ival<numberOfSelectedRows; ++ival) {
      values.GetMatrixArray()[nfill++] = arr[ival];
    }
  }

}

//_____________________________________________________________________________
Double_t TStatToolkit::GetDistance(const TVectorD &values, const ENormType normType,
                                   const Bool_t normaliseToEntries/*=kFALSE*/, const Double_t pvalue/*=1*/)
{
  /// Calculate the distance of the elements in values using a certain norm
  /// \param[in] values             array with input values
  /// \param[in] normType           normalisation to use
  /// \param[in] normaliseToEntries divide the norm by the number of eleements ('average norm')
  /// \param[in] pvalue             the p value for the p-type norm, ignored for all other norms
  /// \return                       calculated distance

  Double_t norm=0.;

  switch (normType) {
    case kL1:
      norm=values.Norm1();
      break;
    case kL2:
      norm=TMath::Sqrt(values.Norm2Sqr());
      break;
    case kLp:
    {
      if (pvalue<1.) {
        ::Error("TStatToolkit::GetDistance","Lp norm: p-value=%5.3g not valid. Only p-value>=1 is allowed", pvalue);
        break;
      }
      Double_t sum=0.;
      for (Int_t ival=0; ival<values.GetNrows(); ++ival) {
        sum+=TMath::Power(TMath::Abs(values.GetMatrixArray()[ival]), pvalue);
      }
      norm=TMath::Power(sum, 1./pvalue);
    }
    break;
    case kMax:
      norm=values.NormInf();
      break;
    case kHamming:
    {
      Double_t sum=0.;
      for (Int_t ival=0; ival<values.GetNrows(); ++ival) {
        if (TMath::Abs(values.GetMatrixArray()[ival])>1e-30) ++sum;
      }
      norm=sum;
    }
    break;
    default:
      return 0;
  }
  if (normaliseToEntries && values.GetNrows()>0) {
    norm/=values.GetNrows();
  }
  return norm;
}

//_____________________________________________________________________________
Double_t TStatToolkit::GetDistance(const Int_t size, const Double_t *values, const ENormType normType,
                                   const Bool_t normaliseToEntries/*=kFALSE*/, const Double_t pvalue/*=1*/)
{
  /// Calculate the distance of the elements in values using a certain norm
  /// \sa GetDistance()
  TVectorD vecvalues;
  vecvalues.Use(size, values);
  return GetDistance(vecvalues, normType, normaliseToEntries, pvalue);
}

//_____________________________________________________________________________
Double_t TStatToolkit::GetDistance(TTree * tree, const char* var, const char * selection,
                                   const ENormType normType, const Bool_t normaliseToEntries/*=kFALSE*/, const Double_t pvalue/*=1*/)
{
  /// Calculate the distance of the values selecte in tree->Draw(var, selection)
  ///
  /// If var contains more than one variable (separated by ':' as usual) the arrays
  /// are concatenated:<BR>
  /// E.g. if var="v1:v2:v3", then the norm of the
  /// the concatenated array of the values from v1,v2 and v3 will be calculated:<BR>
  /// This means if the internal tree arrays for each variable are v1[0..n], v2[0..n], v3[0..n] then
  /// the norm of vx[0..3n] = [v1, v2, v3] is calculated.
  /// \param[in] tree               input tree
  /// \param[in] var                variable expression for the tree->Draw()
  /// \param[in] selection          selection for the tree->Draw()
  /// \param[in] normType           norm to use for calculating the point distances
  /// \param[in] normaliseToEntries divide the norm by the number of eleements ('average norm')
  /// \param[in] pvalue             p-value for the p-norm (ignored for other norm types
  /// \return                       calculated distnace
  Int_t entries = tree->Draw(var,selection,"goff");
  if (entries==0) return 0.;

  TVectorD values;
  CombineArray(tree, values);
  return GetDistance(values, normType, normaliseToEntries, pvalue);
}



void TStatToolkit::MakeDistortionMap(Int_t iter, THnBase * histo, TTreeSRedirector *pcstream, TMatrixD &projectionInfo,Int_t dumpHisto, Int_t verbose){
  //
  // Recursive function to calculate Distortion maps from the residual histograms
  // Input:
  //   iter     - ndim..0
  //   histo    - THn histogram
  //   pcstream -
  //   projectionInfo  - TMatrix speicifiing distortion map cration setup
  //     user specify columns:
  //       0.) sequence of dimensions 
  //       1.) grouping in dimensions (how many bins will be groupd in specific dimension - 0 means onl specified bin 1, curren +-1 bin ...)
  //       2.) step in dimension ( in case >1 some n(projectionInfo(<dim>,2) bins will be not exported
  //     internally used collumns (needed to pass current bin index and bin center to the recursive function) 
  //       3.) current bin value  
  //       4.) current bin center
  //
  //  Output:
  //   pcstream - file with output distortion tree
  //    1.) distortion characteristic: mean, rms, gaussian fit parameters, meang, rmsG chi2 ... at speciefied bin 
  //    2.) specidfied bins (tree branches) are defined by the name of the histogram axis in input histograms
  //  
  //    
  //   Example projection info
  /*
    TFile *f  = TFile::Open("/hera/alice/hellbaer/alice-tpc-notes/SpaceChargeDistortion/data/ATO-108/fullMerge/SCcalibMergeLHC12d.root");
    THnF* histof= (THnF*) f->Get("deltaY_ClTPC_ITSTOF");
    histof->SetName("deltaRPhiTPCTISTOF");
    histof->GetAxis(4)->SetName("qpt");
    TH1::SetDirectory(0);
    TTreeSRedirector * pcstream = new TTreeSRedirector("distortion.root","recreate");
    TMatrixD projectionInfo(5,5);
    projectionInfo(0,0)=0;  projectionInfo(0,1)=0;  projectionInfo(0,2)=0;
    projectionInfo(1,0)=4;  projectionInfo(1,1)=0;  projectionInfo(1,2)=1; 
    projectionInfo(2,0)=3;  projectionInfo(2,1)=3;  projectionInfo(2,2)=2;
    projectionInfo(3,0)=2;  projectionInfo(3,1)=0;  projectionInfo(3,2)=5;
    projectionInfo(4,0)=1;  projectionInfo(4,1)=5;  projectionInfo(4,2)=20;
    MakeDistortionMap(4, histof, pcstream, projectionInfo); 
    delete pcstream;
  */
  //
  static TF1 fgaus("fgaus","gaus",-10,10);
  const Double_t kMinEntries=50;
  Int_t ndim=histo->GetNdimensions();
//  Int_t axis[ndim];
  Double_t meanVector[ndim];
  Int_t binVector[ndim];
  Double_t centerVector[ndim];
//  for (Int_t idim=0; idim<ndim; idim++) axis[idim]=idim;
  //
  if (iter==0){
    char tname[100];
    char aname[100];
    char bname[100];
    char cname[100];
    
    snprintf(tname, 100, "%sDist",histo->GetName());
    //
    //
    // 1.) Calculate  properties   - mean, rms, gaus fit, chi2, entries
    // 2.) Dump properties to tree 1D properties  - plus dimension descriptor f
    Int_t dimProject   = TMath::Nint(projectionInfo(iter,0));
    TH1 *his1DFull = histo->Projection(dimProject);
    Double_t mean= his1DFull->GetMean();
    Double_t rms= his1DFull->GetRMS();
    Int_t entries=  his1DFull->GetEntries();
    TString hname="his_";
    for (Int_t idim=0; idim<ndim; idim++) {hname+="_"; hname+=TMath::Nint(projectionInfo(idim,3));}
    Double_t meanG=0, rmsG=0, chi2G=0;
    if (entries>kMinEntries){
      fgaus.SetParameters(entries,mean,rms);
      his1DFull->Fit(&fgaus,"qnr","qnr");
      meanG = fgaus.GetParameter(1);
      rmsG = fgaus.GetParameter(2);
      chi2G = fgaus.GetChisquare()/fgaus.GetNumberFreeParameters();
    }
    if (dumpHisto>=0) {
      static Int_t histoCounter=0;
      if ((histoCounter%dumpHisto)==0) his1DFull->Write(hname.Data());
      histoCounter++;
    }
    delete his1DFull;
    (*pcstream)<<tname<<
    "entries="<<entries<< // number of entries
    "mean="<<mean<<       // mean value of the last dimension
    "rms="<<rms<<         // rms value of the last dimension
    "meanG="<<meanG<<     // mean of the gaus fit
    "rmsG="<<rmsG<<       // rms of the gaus fit
    "chi2G="<<chi2G;      // chi2 of the gaus fit
    
    for (Int_t idim=0; idim<ndim; idim++){
      TH1 *his1DAxis = histo->Projection(idim);
      meanVector[idim] = his1DAxis->GetMean();
      snprintf(aname, 100, "%sMean=",histo->GetAxis(idim)->GetName());
      (*pcstream)<<tname<<
      aname<<meanVector[idim];      // current bin means
      delete his1DAxis;
    }
    for (Int_t iIter=0; iIter<ndim; iIter++){
      Int_t idim = TMath::Nint(projectionInfo(iIter,0));
      binVector[idim] = TMath::Nint(projectionInfo(iIter,3));
      centerVector[idim] = projectionInfo(iIter,4);
      snprintf(bname, 100, "%sBin=",histo->GetAxis(idim)->GetName());
      snprintf(cname, 100, "%sCenter=",histo->GetAxis(idim)->GetName());
      (*pcstream)<<tname<<
      bname<<binVector[idim]<<      // current bin values
      cname<<centerVector[idim];    // current bin centers
    }
    (*pcstream)<<tname<<"\n";
  }else{
    // loop over the diminsion of interest
    //      project selecting bin+-deltabin histoProj
    //      MakeDistortionMap(histoProj ...) 
    //
    Int_t dimProject   = TMath::Nint(projectionInfo(iter,0));
    Int_t groupProject =  TMath::Nint(projectionInfo(iter,1));
    Int_t stepProject =  TMath::Nint(projectionInfo(iter,2));
    if (stepProject<1) stepProject=1;
    Int_t nbins = histo->GetAxis(dimProject)->GetNbins();
    
    for (Int_t ibin=1; ibin<=nbins; ibin+=stepProject){
      if (iter>1 && verbose){
        for (Int_t idim=0; idim<ndim; idim++){
          printf("\t%d(%d,%d)",TMath::Nint(projectionInfo(idim,3)),TMath::Nint(projectionInfo(idim,0)),TMath::Nint(projectionInfo(idim,1) ));
        }
        printf("\n");	    
      }
      Int_t bin0=TMath::Max(ibin-groupProject,1);
      Int_t bin1=TMath::Min(ibin+groupProject,nbins);
      histo->GetAxis(dimProject)->SetRange(bin0,bin1);
      projectionInfo(iter,3)=ibin;
      projectionInfo(iter,4)=histo->GetAxis(dimProject)->GetBinCenter(ibin);
      Int_t iterProject=iter-1;
      MakeDistortionMap(iterProject, histo, pcstream, projectionInfo);
    }
  }
  //
}

void TStatToolkit::MakeDistortionMapFast(THnBase * histo, TTreeSRedirector *pcstream, TMatrixD &projectionInfo,Int_t verbose,  Double_t fractionCut, const char * estimators)
{
  //
  // Function to calculate Distortion maps from the residual histograms
  // Input:
  //   histo    - THn histogram
  //   pcstream -
  //   projectionInfo  - TMatrix specifying distortion map creation setup
  //     user specify columns:
  //       0.) sequence of dimensions 
  //       1.) grouping in dimensions (how many bins will be grouped in specific dimension - 0 means onl specified bin 1, current +-1 bin ...)
  //       2.) step in dimension ( in case >1 some n(projectionInfo(<dim>,2) bins will be not exported
  //
  //  Output:
  //   pcstream - file with output distortion tree
  //    1.) distortion characteristic: mean, rms, gaussian fit parameters, meanG, rmsG chi2 ... at specified bin
  //    2.) specified bins (tree branches) are defined by the name of the histogram axis in input histograms
  //    3.) in debug mode - controlled by env variable "gDumpHistoFraction" fraction of histogram + fits dumped to the file
  //    
  //   Example projection info
  /*
    TFile *f  = TFile::Open("/hera/alice/hellbaer/alice-tpc-notes/SpaceChargeDistortion/data/ATO-108/fullMerge/SCcalibMergeLHC12d.root");
    THnF* histoF= (THnF*) f->Get("deltaY_ClTPC_ITSTOF");
    histoF->SetName("deltaRPhiTPCTISTOF");
    histoF->GetAxis(4)->SetName("qpt");
    TH1::SetDirectory(0);
    TTreeSRedirector * pcstream = new TTreeSRedirector("distortion.root","recreate");
    TMatrixD projectionInfo(5,3);
    projectionInfo(0,0)=0;  projectionInfo(0,1)=0;  projectionInfo(0,2)=0;
    projectionInfo(1,0)=4;  projectionInfo(1,1)=0;  projectionInfo(1,2)=1; 
    projectionInfo(2,0)=3;  projectionInfo(2,1)=3;  projectionInfo(2,2)=2;
    projectionInfo(3,0)=2;  projectionInfo(3,1)=0;  projectionInfo(3,2)=5;
    projectionInfo(4,0)=1;  projectionInfo(4,1)=5;  projectionInfo(4,2)=20;
    MakeDistortionMap(histoF, pcstream, projectionInfo);
    delete pcstream;
  */
  //
  if (histo->GetNdimensions()<=1) {
    ::Error("TStatToolkit::MakeDistortionMapFast","Invalid dimension of input histogram");
    return;
  }
  if (fractionCut<0 || fractionCut>0.4){
    ::Error("TStatToolkit::MakeDistortionMapFast","Invalid input fraction cut %f\r. Should be in range <0,0.4>", fractionCut);
    return;
  }
  const Double_t kMinEntries=30, kUseLLFrom=20;
  const Float_t  kDumpHistoFraction = TString(gSystem->Getenv("gDumpHistoFraction")).Atof();  // in debug mode - controlled by env variable "gDumpHistoFraction" fractio of histogram + fits dumped to the file 
  char tname[100];
  char aname[100];
  char bname[100];
  char cname[100];
  Float_t fractionLTM[100]={0.8};
  TVectorF *vecLTM[100]={0};
  Int_t nestimators=1;
  if (estimators!=NULL){
    TObjArray * array=TString(estimators).Tokenize(":");
    nestimators=array->GetEntries();
    for (Int_t iest=0; iest<nestimators; iest++){
      fractionLTM[iest]=TString(array->At(iest)->GetName()).Atof();
    }
  }
  for (Int_t iest=0; iest<nestimators; iest++) {
    vecLTM[iest]=new TVectorF(10);
    (*(vecLTM[iest]))[9]= fractionLTM[iest];
  }

  //
  int ndim = histo->GetNdimensions();
  int nbins[ndim],idx[ndim],idxmin[ndim],idxmax[ndim],idxSav[ndim];
  for (int id=0;id<ndim;id++) nbins[id] = histo->GetAxis(id)->GetNbins();
  //
  int axOrd[ndim],binSt[ndim],binGr[ndim];
  for (int i=0;i<ndim;i++) {
    axOrd[i] = TMath::Nint(projectionInfo(i,0));
    binGr[i] = TMath::Nint(projectionInfo(i,1));
    binSt[i] = TMath::Max(1,TMath::Nint(projectionInfo(i,2)));
  }
  int tgtDim = axOrd[0],tgtStep=binSt[0],tgtNb=nbins[tgtDim],tgtNb1=tgtNb+1;
  double binY[tgtNb],binX[tgtNb],meanVector[ndim],centerVector[ndim], meanVectorMI[ndim+2];
  Int_t binVector[ndim];
  // prepare X axis
  TAxis* xax = histo->GetAxis(tgtDim);
  for (int i=tgtNb;i--;) binX[i] = xax->GetBinCenter(i+1);
  for (int i=ndim;i--;) idx[i]=1;
  Bool_t grpOn = kFALSE;
  for (int i=1;i<ndim;i++) if (binGr[i]) grpOn = kTRUE;
  //
  // estimate number of output fits
  histo->GetListOfAxes()->Print();
  ULong64_t nfits = 1, fitCount=0;
  printf("index\tdim\t|\tnbins\tgrouping\tstep\tnfits\n");
  for (int i=1;i<ndim;i++) {
    int idim = axOrd[i];
    nfits *= TMath::Max(1,nbins[idim]/binSt[idim]);
    printf("%d %d | %d %d %d %lld\n",i,idim,nbins[idim],binGr[idim], binSt[idim],nfits);
  }
  printf("Expect %lld nfits\n",nfits);
  ULong64_t fitProgress = nfits/100;
  //
  // setup fit function, at the moment full root fit
  static TF1 fgaus("fgaus","gaus",-10,10);
  fgaus.SetRange(xax->GetXmin(),xax->GetXmax());
  //  TGraph grafFit(tgtNb);
  TH1F* hfit = new TH1F("hfit","hfit",tgtNb,xax->GetXmin(),xax->GetXmax());
  //
  snprintf(tname, 100, "%sDist",histo->GetName());
  TStopwatch sw;
  sw.Start();
  int dimVar=1, dimVarID = axOrd[dimVar];
  //
  //  TVectorF  vecLTM(9);
  while(1) {
    //
    //double dimVarCen = histo->GetAxis(dimVarID)->GetBinCenter(idx[dimVarID]); // center of currently varied bin
    //
    if (grpOn) { //>> grouping requested?
      memset(binY,0,tgtNb*sizeof(double)); // need to accumulate
      //
      for (int idim=1;idim<ndim;idim++) {
        int grp = binGr[idim];
        int idimR = axOrd[idim]; // real axis id
        idxSav[idimR]=idx[idimR]; // save central bins
        idxmax[idimR] = TMath::Min(idx[idimR]+grp,nbins[idimR]);
        idx[idimR] = idxmin[idimR] = TMath::Max(1,idx[idimR]-grp);
        //
        // effective bin center
        meanVector[idimR] = 0;
        TAxis* ax = histo->GetAxis(idimR);
        if (grp>0) {
          for (int k=idxmin[idimR];k<=idxmax[idimR];k++) meanVector[idimR] += ax->GetBinCenter(k);
          meanVector[idimR] /= (1+(grp<<1));
        }
        else meanVector[idimR] = ax->GetBinCenter(idxSav[idimR]);
      } // set limits for grouping
      if (verbose>0) {
        printf("output bin: ");
        for (int i=0;i<ndim;i++) if (i!=tgtDim) printf("[D:%d]:%d ",i,idxSav[i]);
        printf("\n");
        printf("integrates: ");
        for (int i=0;i<ndim;i++) if (i!=tgtDim) printf("[D:%d]:%d-%d ",i,idxmin[i],idxmax[i]);
        printf("\n");
      }
      //
      for (Int_t jDim=0; jDim<ndim+2; jDim++) meanVectorMI[jDim]=0;
      while(1) {
        // loop over target dimension: accumulation
        int &it = idx[tgtDim];
        for (it=1;it<tgtNb1;it+=tgtStep) {
          Double_t content=histo->GetBinContent(idx);
          binY[it-1] += content;
          for (Int_t jDim=0; jDim<ndim; jDim++) meanVectorMI[jDim]+=content*histo->GetAxis(jDim)->GetBinCenter(idx[jDim]);
          meanVectorMI[ndim]+=content;
          meanVectorMI[ndim+1]+=1.;
          if (verbose>1) {for (int i=0;i<ndim;i++) printf("%d ",idx[i]); printf(" | accumulation\n");}
        }
        //
        int idim;
        for (idim=1;idim<ndim;idim++) { // dimension being groupped
          int idimR = axOrd[idim]; // real axis id in the histo
          if ( (++idx[idimR]) > idxmax[idimR] ) idx[idimR]=idxmin[idimR];
          else break;
        }
        if (idim==ndim) break;
      }
    } // <<grouping requested
    else {
      int &it = idx[tgtDim];
      for (it=1;it<tgtNb1;it+=tgtStep) {
        binY[it-1] = histo->GetBinContent(idx);
        //
        //for (int i=0;i<ndim;i++) printf("%d ",idx[i]); printf(" | \n");
      }
      for (int idim=1;idim<ndim;idim++) {
        int idimR = axOrd[idim]; // real axis id
        meanVector[idimR] = histo->GetAxis(idimR)->GetBinCenter(idx[idimR]);
      }
    }
    if (grpOn) for (int i=ndim;i--;) idx[i]=idxSav[i]; // restore central bins
    idx[tgtDim] = 0;
    if (verbose>0) {for (int i=0;i<ndim;i++) printf("%d ",idx[i]); printf(" | central bin fit\n");}
    if (meanVectorMI[ndim]>1) { //TODO check valgrind
      for (Int_t jDim=0; jDim<ndim; jDim++) meanVectorMI[jDim]/= meanVectorMI[ndim];
    }else{
      for (Int_t jDim=0; jDim<ndim; jDim++) meanVectorMI[jDim]= meanVector[ndim];
    }
    // 
    // >> ------------- do fit
    float mean=0,mom2=0,rms=0,m3=0, m4=0, nrm=0,meanG=0,rmsG=0,chi2G=0,maxVal=0,entriesG=0,mean0=0, rms0=0;
    hfit->Reset();
    for (int ip=tgtNb;ip--;) {
      //grafFit.SetPoint(ip,binX[ip],binY[ip]);
      hfit->SetBinContent(ip+1,binY[ip]);
      nrm  += binY[ip];
      mean += binX[ip]*binY[ip];
      mom2 += binX[ip]*binX[ip]*binY[ip];
      if (maxVal<binY[ip]) maxVal = binY[ip];
    }
    if (nrm>0) {
      mean /= nrm;
      mom2 /= nrm;
      rms = mom2 - mean*mean;
      rms = rms>0 ? TMath::Sqrt(rms):0;
    }
    mean0=mean;
    rms0=rms;


    Int_t nbins1D=hfit->GetNbinsX();
    Float_t binMedian=0;
    Double_t limits[2]={hfit->GetBinCenter(1), hfit->GetBinCenter(nbins1D)};
    if (nrm>5) {
      for (Int_t iest=0; iest<nestimators; iest++){
        TStatToolkit::LTMHisto(hfit, *(vecLTM[iest]), fractionLTM[iest]);
      }
      Double_t* integral=hfit->GetIntegral();
      for (Int_t i=1; i<nbins1D-1; i++){
        if (integral[i-1]<0.5 && integral[i]>=0.5){
          if (hfit->GetBinContent(i-1)+hfit->GetBinContent(i)>0){
            binMedian=hfit->GetBinCenter(i);
            Double_t dIdx=-(integral[i-1]-integral[i]);
            Double_t dx=(0.5+(0.5-integral[i])/dIdx)*hfit->GetBinWidth(i);
            binMedian+=dx;
          }
        }
        if (integral[i-1]<fractionCut && integral[i]>=fractionCut){
          limits[0]=hfit->GetBinCenter(i-1)-hfit->GetBinWidth(i);
        }
        if (integral[i]<1-fractionCut && integral[i+1]>=1-fractionCut){
          limits[1]=hfit->GetBinCenter(i+1)+hfit->GetBinWidth(i);
        }
      }
    }
    if (nrm>5&&fractionCut>0 &&rms>0) {
      hfit->GetXaxis()->SetRangeUser(limits[0], limits[1]);
      mean=hfit->GetMean();
      rms=hfit->GetRMS();
      if (nrm>0 && rms>0) {
        m3=hfit->GetSkewness();
        m4=hfit->GetKurtosis();
      }
      // fgaus.SetRange(limits[0]-rms, limits[1]+rms);   //TODO - to fix bug in range limits
    }else{
      fgaus.SetRange(xax->GetXmin(),xax->GetXmax());
    }


    Bool_t isFitValid=kFALSE;
    if (nrm>=kMinEntries && rms>hfit->GetBinWidth(nbins1D)/TMath::Sqrt(12.)) {
      fgaus.SetParameters(nrm/(rms/hfit->GetBinWidth(nbins1D)),mean,rms);
      fgaus.SetParError(0,nrm/(rms/hfit->GetBinWidth(nbins1D)));
      fgaus.SetParError(1,rms);
      fgaus.SetParError(2,rms);
      //grafFit.Fit(&fgaus,/*maxVal<kUseLLFrom ? "qnrl":*/"qnr");
      TFitResultPtr fitPtr= hfit->Fit(&fgaus,maxVal<kUseLLFrom ? "qnrlS+":"qnrS+");
      //TFitResultPtr fitPtr= hfit->Fit(&fgaus,"qnrlS");
      entriesG = fgaus.GetParameter(0);
      meanG = fgaus.GetParameter(1);
      rmsG  = fgaus.GetParameter(2);
      chi2G = fgaus.GetChisquare()/fgaus.GetNumberFreeParameters();
      TFitResult * result = fitPtr.Get();
      if (result!=NULL){
        isFitValid = result->IsValid();
      }
      //
    }
    TH1 * hDump=0;
    if (nrm>=kMinEntries&& kDumpHistoFraction>0 && (gRandom->Rndm()<kDumpHistoFraction ||  isFitValid!=kTRUE)){
      hDump=hfit;
    }
    if (kDumpHistoFraction>=1.){ /// dump all histograms fraction>=1 independent of the status
      hDump=hfit;
    }
    if (hDump){
      hfit->GetListOfFunctions()->AddLast(&fgaus);
      (*pcstream)<<TString::Format("%sDump", tname).Data()<<
                 "entries="<<nrm<<     // number of entries
                 "isFitValid="<<isFitValid<< // true if the gaus fit converged
                 "hDump.="<<hDump<<    // histogram  - by default not filled
                 "mean0="<<mean0<<       // mean value of the last dimension - without fraction cut
                 "rms0="<<rms0<<         // rms value of the last dimension - without fraction cut
                 "mean="<<mean<<       // mean value of the last dimension
                 "rms="<<rms<<         // rms value of the last dimension
                 "m3="<<m3<<            // m3 (skewnes) of the last dimension
                 "m4="<<m4<<            // m4 (kurtosis) of the last dimension
                 "binMedian="<<binMedian<< //binned median value of 1D histogram
                 "entriesG="<<entriesG<<
                 "meanG="<<meanG<<     // mean of the gaus fit
                 "rmsG="<<rmsG<<       // rms of the gaus fit
                 "vecLTM.="<<vecLTM[0]<<   // LTM  frac% statistic
                 "chi2G="<<chi2G;      // chi2 of the gaus fit
      for (Int_t iest=1; iest<nestimators; iest++)
        (*pcstream)<<TString::Format("%sDump", tname).Data()<<TString::Format("vecLTM%d.=",iest)<<vecLTM[iest];   // LTM  frac% statistic

    }

    (*pcstream)<<tname<<
               "entries="<<nrm<<     // number of entries
               "isFitValid="<<isFitValid<< // true if the gaus fit converged
               "mean0="<<mean0<<       // mean value of the last dimension - without fraction cut
               "rms0="<<rms0<<         // rms value of the last dimension - without fraction cut
               "mean="<<mean<<       // mean value of the last dimension
               "rms="<<rms<<         // rms value of the last dimension
               "m3="<<m3<<            // m3 (skewnes) of the last dimension
               "m4="<<m4<<            // m4 (kurtosis) of the last dimension
               "binMedian="<<binMedian<< //binned median value of 1D histogram
               "entriesG="<<entriesG<<   //
               "meanG="<<meanG<<     // mean of the gaus fit
               "rmsG="<<rmsG<<       // rms of the gaus fit
               "vecLTM.="<<vecLTM[0]<<   // LTM  frac% statistic
               "chi2G="<<chi2G;      // chi2 of the gaus fit
    for (Int_t iest=1; iest<nestimators; iest++)
      (*pcstream)<<tname<<TString::Format("vecLTM%d.=",iest)<<vecLTM[iest];   // LTM  frac% statistic


    //
    meanVector[tgtDim] = mean; // what's a point of this?
    for (Int_t idim=0; idim<ndim; idim++){
      snprintf(aname, 100, "%sMean=",histo->GetAxis(idim)->GetName());
      (*pcstream)<<tname<<
                 aname<<meanVectorMI[idim];      // current bin means
    }
    //
    for (Int_t iIter=0; iIter<ndim; iIter++){
      Int_t idim = axOrd[iIter];
      binVector[idim] = idx[idim];
      centerVector[idim] = histo->GetAxis(idim)->GetBinCenter(idx[idim]);
      snprintf(bname, 100, "%sBin=",histo->GetAxis(idim)->GetName());
      snprintf(cname, 100, "%sCenter=",histo->GetAxis(idim)->GetName());
      (*pcstream)<<tname<<
                 bname<<binVector[idim]<<      // current bin values
                 cname<<centerVector[idim];    // current bin centers
      if (hDump){
        (*pcstream)<<TString::Format("%sDump", tname).Data()<<
                   bname<<binVector[idim]<<      // current bin values
                   cname<<centerVector[idim];    // current bin centers
      }
    }
    (*pcstream)<<tname<<"\n";
    if (hDump)	{
      (*pcstream)<<TString::Format("%sDump", tname).Data()<<"\n";
      hfit->GetListOfFunctions()->RemoveLast();
    }
    // << ------------- do fit
    //
    if ( fitProgress>0 && nfits>0) {
      if   (((++fitCount)%fitProgress)==0) {
        printf("fit %lld %4.1f%% done\n",fitCount,100*double(fitCount)/nfits);
      }
    }
    //
    //next global bin in which target dimention will be looped
    for (dimVar=1;dimVar<ndim;dimVar++) { // varying dimension
      dimVarID = axOrd[dimVar]; // real axis id in the histo
      if ( (idx[dimVarID]+=binSt[dimVar]) > nbins[dimVarID] ) idx[dimVarID]=1;
      else break;
    }
    if (dimVar==ndim) break;
  }
  delete hfit;
  sw.Stop();
  sw.Print();
  /*
  int nb = histo->GetNbins();
  int prc = nb/100;
  for (int i=0;i<nb;i++) {
    histo->GetBinContent(i);
    if (i && (i%prc)==0) printf("Done %d%%\n",int(float(100*i)/prc));
  }
  */
}


/// TStatToolkit::MakePDFMap function to calculate statistics form the N dimensnal PDF map
/// Original implementation - a copy of the MakeDistortionMapFast
/// \param histo              -  input n dimsnional histogram
/// \param pcstream           -  output stream to store tree with PDF statistic maps
/// \param projectionInfo     -
/// \param options            - option - parameterize statistic to extract
/// \param verbose            - verbosity of extraction
/// Example:
/// options["exportGraph"]="1";
///  options["exportGraphCumulative"]="1";
///  options["LTMestimators"]="0.6:0.5:0.4";
//  options["LTMFitRange"]="0.6:5:1";
void TStatToolkit::MakePDFMap(THnBase *histo, TTreeSRedirector *pcstream, TMatrixD &projectionInfo, std::map<std::string, std::string> pdfOptions, Int_t verbose)
{
  //
  if (histo->GetNdimensions()<=1) {
    ::Error("TStatToolkit::MakeDistortionMapFast","Invalid dimension of input histogram");
    return;
  }
  // Parse options
  Float_t fractionCut=0;
  string estimators="";
  Float_t kDumpHistoFraction=0;
  Bool_t exportGraph=kFALSE;
  Bool_t exportGraphCumulative=kFALSE;
  Bool_t LTMFitRange=kFALSE;
  Float_t LTMFitInfo[3]={1.0,6,0};
  TGraphErrors *graphHisto=0;
  TGraphErrors *graphCumulative=0;

  if ( pdfOptions[string("fractionCut")] != "" ) fractionCut = atof(pdfOptions[string("fractionCut")].data());
  if ( pdfOptions[string("LTMestimators")] != "" ) estimators = pdfOptions[string("LTMestimators")];
  if ( pdfOptions[string("DumpHistFraction")] != "" ) kDumpHistoFraction = atof(pdfOptions[string("DumpHistFraction")].data());
  if ( pdfOptions[string("exportGraph")] == "1" )      exportGraph=kTRUE;
  if ( pdfOptions[string("exportGraphCumulative")] == "1" ) exportGraphCumulative=kTRUE;
  if ( pdfOptions[string("LTMFitRange")] != "" ) LTMFitRange=kTRUE;

  // Check options
  if (fractionCut<0 || fractionCut>0.4){
    ::Error("TStatToolkit::MakeDistortionMapFast","Invalid input fraction cut %f\r. Should be in range <0,0.4>", fractionCut);
    return;
  }
  const Double_t kMinEntries=30, kUseLLFrom=20, kMinStatLTM=5;
  char tname[100];
  char aname[100];
  char bname[100];
  char cname[100];
  Float_t fractionLTM[100]={0.8};
  TVectorF *vecLTM[100]={0};
  Int_t nestimators=1;
  if (estimators!=""){
    TObjArray * array=TString(estimators).Tokenize(":");
    nestimators=array->GetEntries();
    for (Int_t iest=0; iest<nestimators; iest++){
      fractionLTM[iest]=TString(array->At(iest)->GetName()).Atof();
    }
  }
  if (LTMFitRange){
    TObjArray * array=TString(pdfOptions[string("LTMFitRange")]).Tokenize(":");
    Int_t nInfo=array->GetEntries();
    for (Int_t i=0; i<nInfo; i++){
      LTMFitInfo[i]=TString(array->At(i)->GetName()).Atof();
    }
    fractionLTM[nestimators]=LTMFitInfo[0];
    nestimators++;
  }
  for (Int_t iest=0; iest<nestimators; iest++) {
    vecLTM[iest]=new TVectorF(10);
    (*(vecLTM[iest]))[9]= fractionLTM[iest];
  }

  //
  int ndim = histo->GetNdimensions();
  int nbins[ndim],idx[ndim],idxmin[ndim],idxmax[ndim],idxSav[ndim];
  for (int id=0;id<ndim;id++) nbins[id] = histo->GetAxis(id)->GetNbins();
  //
  int axOrd[ndim],binSt[ndim],binGr[ndim];
  for (int i=0;i<ndim;i++) {
    axOrd[i] = TMath::Nint(projectionInfo(i,0));
    binGr[i] = TMath::Nint(projectionInfo(i,1));
    binSt[i] = TMath::Max(1,TMath::Nint(projectionInfo(i,2)));
  }
  int tgtDim = axOrd[0],tgtStep=binSt[0],tgtNb=nbins[tgtDim],tgtNb1=tgtNb+1;
  double binY[tgtNb],binX[tgtNb],meanVector[ndim],centerVector[ndim], meanVectorMI[ndim+2];
  Int_t binVector[ndim];
  // prepare X axis
  TAxis* xax = histo->GetAxis(tgtDim);
  for (int i=tgtNb;i--;) binX[i] = xax->GetBinCenter(i+1);
  for (int i=ndim;i--;) idx[i]=1;
  Bool_t grpOn = kFALSE;
  for (int i=1;i<ndim;i++) if (binGr[i]) grpOn = kTRUE;
  //
  // estimate number of output fits
  histo->GetListOfAxes()->Print();
  ULong64_t nfits = 1, fitCount=0;
  printf("index\tdim\t|\tnbins\tgrouping\tstep\tnfits\n");
  for (int i=1;i<ndim;i++) {
    int idim = axOrd[i];
    nfits *= TMath::Max(1,nbins[idim]/binSt[idim]);
    printf("%d %d | %d %d %d %lld\n",i,idim,nbins[idim],binGr[idim], binSt[idim],nfits);
  }
  printf("Expect %lld nfits\n",nfits);
  ULong64_t fitProgress = nfits/100;
  //
  // setup fit function, at the moment full root fit
  TF1 fgaus("fgaus","gaus",xax->GetXmin(),xax->GetXmax());
  fgaus.SetRange(xax->GetXmin(),xax->GetXmax());
  //  TGraph grafFit(tgtNb);
  TH1F* hfit = new TH1F("hfit","hfit",tgtNb,xax->GetXmin(),xax->GetXmax());
  //
  snprintf(tname, 100, "%sDist",histo->GetName());
  TStopwatch sw;
  sw.Start();
  int dimVar=1, dimVarID = axOrd[dimVar];
  //
  //  TVectorF  vecLTM(9);
  while(1) {
    //
    //double dimVarCen = histo->GetAxis(dimVarID)->GetBinCenter(idx[dimVarID]); // center of currently varied bin
    //
    if (grpOn) { //>> grouping requested?
      memset(binY,0,tgtNb*sizeof(double)); // need to accumulate
      //
      for (int idim=1;idim<ndim;idim++) {
        int grp = binGr[idim];
        int idimR = axOrd[idim]; // real axis id
        idxSav[idimR]=idx[idimR]; // save central bins
        idxmax[idimR] = TMath::Min(idx[idimR]+grp,nbins[idimR]);
        idx[idimR] = idxmin[idimR] = TMath::Max(1,idx[idimR]-grp);
        //
        // effective bin center
        meanVector[idimR] = 0;
        TAxis* ax = histo->GetAxis(idimR);
        if (grp>0) {
          for (int k=idxmin[idimR];k<=idxmax[idimR];k++) meanVector[idimR] += ax->GetBinCenter(k);
          meanVector[idimR] /= (1+(grp<<1));
        }
        else meanVector[idimR] = ax->GetBinCenter(idxSav[idimR]);
      } // set limits for grouping
      if (verbose>0) {
        printf("output bin: ");
        for (int i=0;i<ndim;i++) if (i!=tgtDim) printf("[D:%d]:%d ",i,idxSav[i]);
        printf("\n");
        printf("integrates: ");
        for (int i=0;i<ndim;i++) if (i!=tgtDim) printf("[D:%d]:%d-%d ",i,idxmin[i],idxmax[i]);
        printf("\n");
      }
      //
      for (Int_t jDim=0; jDim<ndim+2; jDim++) meanVectorMI[jDim]=0;
      while(1) {
        // loop over target dimension: accumulation
        int &it = idx[tgtDim];
        for (it=1;it<tgtNb1;it+=tgtStep) {
          Double_t content=histo->GetBinContent(idx);
          binY[it-1] += content;
          for (Int_t jDim=0; jDim<ndim; jDim++) meanVectorMI[jDim]+=content*histo->GetAxis(jDim)->GetBinCenter(idx[jDim]);
          meanVectorMI[ndim]+=content;
          meanVectorMI[ndim+1]+=1.;
          if (verbose>1) {for (int i=0;i<ndim;i++) printf("%d ",idx[i]); printf(" | accumulation\n");}
        }
        //
        int idim;
        for (idim=1;idim<ndim;idim++) { // dimension being groupped
          int idimR = axOrd[idim]; // real axis id in the histo
          if ( (++idx[idimR]) > idxmax[idimR] ) idx[idimR]=idxmin[idimR];
          else break;
        }
        if (idim==ndim) break;
      }
    } // <<grouping requested
    else {
      int &it = idx[tgtDim];
      for (it=1;it<tgtNb1;it+=tgtStep) {
        binY[it-1] = histo->GetBinContent(idx);
        //
        //for (int i=0;i<ndim;i++) printf("%d ",idx[i]); printf(" | \n");
      }
      for (int idim=1;idim<ndim;idim++) {
        int idimR = axOrd[idim]; // real axis id
        meanVector[idimR] = histo->GetAxis(idimR)->GetBinCenter(idx[idimR]);
      }
    }
    if (grpOn) for (int i=ndim;i--;) idx[i]=idxSav[i]; // restore central bins
    idx[tgtDim] = 0;
    if (verbose>0) {for (int i=0;i<ndim;i++) printf("%d ",idx[i]); printf(" | central bin fit\n");}
    if (meanVectorMI[ndim]>1) { //TODO check valgrind
      for (Int_t jDim=0; jDim<ndim; jDim++) meanVectorMI[jDim]/= meanVectorMI[ndim];
    }else{
      for (Int_t jDim=0; jDim<ndim; jDim++) meanVectorMI[jDim]= meanVector[ndim];
    }
    //
    // >> ------------- do fit
    float mean=0,mom2=0,rms=0,m3=0, m4=0, nrm=0,meanG=0,rmsG=0,chi2G=0,maxVal=0,entriesG=0,mean0=0, rms0=0,ndf=0;
    hfit->Reset();
    for (int ip=tgtNb;ip--;) {
      //grafFit.SetPoint(ip,binX[ip],binY[ip]);
      hfit->SetBinContent(ip+1,binY[ip]);
      nrm  += binY[ip];
      mean += binX[ip]*binY[ip];
      mom2 += binX[ip]*binX[ip]*binY[ip];
      if (maxVal<binY[ip]) maxVal = binY[ip];
    }
    if (nrm>0) {
      mean /= nrm;
      mom2 /= nrm;
      rms = mom2 - mean*mean;
      rms = rms>0 ? TMath::Sqrt(rms):0;
    }
    mean0=mean;
    rms0=rms;


    Int_t nbins1D=hfit->GetNbinsX();
    Float_t binMedian=0;
    Double_t limits[2]={hfit->GetBinCenter(1), hfit->GetBinCenter(nbins1D)};
    if (nrm>kMinStatLTM) {
      for (Int_t iest=0; iest<nestimators; iest++){
        TStatToolkit::LTMHisto(hfit, *(vecLTM[iest]), fractionLTM[iest]);
      }
      Double_t* integral=hfit->GetIntegral();
      for (Int_t i=1; i<nbins1D-1; i++){
        if (integral[i-1]<0.5 && integral[i]>=0.5){
          if (hfit->GetBinContent(i-1)+hfit->GetBinContent(i)>0){
            binMedian=hfit->GetBinCenter(i);
            Double_t dIdx=-(integral[i-1]-integral[i]);
            Double_t dx=(0.5+(0.5-integral[i])/dIdx)*hfit->GetBinWidth(i);
            binMedian+=dx;
          }
        }
        if (integral[i-1]<fractionCut && integral[i]>=fractionCut){
          limits[0]=hfit->GetBinCenter(i-1)-hfit->GetBinWidth(i);
        }
        if (integral[i]<1-fractionCut && integral[i+1]>=1-fractionCut){
          limits[1]=hfit->GetBinCenter(i+1)+hfit->GetBinWidth(i);
        }
      }
    }
    if (nrm>kMinStatLTM&&fractionCut>0 &&rms>0) {
      hfit->GetXaxis()->SetRangeUser(limits[0], limits[1]);
      mean=hfit->GetMean();
      rms=hfit->GetRMS();
      if (nrm>0 && rms>0) {
        m3=hfit->GetSkewness();
        m4=hfit->GetKurtosis();
      }
      // fgaus.SetRange(limits[0]-rms, limits[1]+rms);   //TODO - to fix bug in range limits
    }else{
      fgaus.SetRange(xax->GetXmin(),xax->GetXmax());
    }
    if (nrm>kMinStatLTM&&LTMFitInfo[1]>0){  /// id specified in options - set fit ranges as nsigma of LTM. if range smaller than 3 bins add bin before and after
      if ((*vecLTM[nestimators-1])[2]*LTMFitInfo[1]>hfit->GetXaxis()->GetBinWidth(0)) {
        Double_t xMin = (*vecLTM[nestimators - 1])[1] - (*vecLTM[nestimators - 1])[2] * LTMFitInfo[1] - LTMFitInfo[2];
        Double_t xMax = (*vecLTM[nestimators - 1])[1] + (*vecLTM[nestimators - 1])[2] * LTMFitInfo[1] + LTMFitInfo[2];
        if (xMax - xMin < 4 * hfit->GetXaxis()->GetBinWidth(0)) {
          xMax += 2. * hfit->GetXaxis()->GetBinWidth(0);
          xMin -= 2. * hfit->GetXaxis()->GetBinWidth(0);
        }
        hfit->GetXaxis()->SetRangeUser(xMin, xMax);
        fgaus.SetRange(xMin, xMax);
      }
    }

    Bool_t isFitValid=kFALSE;
    if (nrm>=kMinEntries && rms>hfit->GetBinWidth(nbins1D)/TMath::Sqrt(12.)) {
      fgaus.SetParameters(nrm/(rms/hfit->GetBinWidth(nbins1D)),mean,rms);
      fgaus.SetParError(0,nrm/(rms/hfit->GetBinWidth(nbins1D)));
      fgaus.SetParError(1,rms);
      fgaus.SetParError(2,rms);
      for (Int_t i=0; i<3; i++) fgaus.SetParLimits(i,0,0);    //in ROOT6 prameters were sometimes stuck - reset limtis
      //grafFit.Fit(&fgaus,/*maxVal<kUseLLFrom ? "qnrl":*/"qnr");
      TFitResultPtr fitPtr= hfit->Fit(&fgaus,maxVal<kUseLLFrom ? "qnrlS+":"qnrS+");
      //TFitResultPtr fitPtr= hfit->Fit(&fgaus,"qnrlS");
      entriesG = fgaus.GetParameter(0);
      meanG = fgaus.GetParameter(1);
      rmsG  = fgaus.GetParameter(2);
      chi2G = fgaus.GetChisquare()/fgaus.GetNDF();
      ndf=fgaus.GetNDF();
      TFitResult * result = fitPtr.Get();
      if (result!=NULL){
        isFitValid = result->IsValid();
      }else {
        isFitValid = kFALSE;
      }
      //
    }
    TH1 * hDump=0;
    if (nrm>=kMinEntries&& kDumpHistoFraction>0 && (gRandom->Rndm()<kDumpHistoFraction ||  isFitValid!=kTRUE)){
      hDump=hfit;
    }
    if (kDumpHistoFraction>=1.){ /// dump all histograms fraction>=1 independent of the status
      hDump=hfit;
    }
    fgaus.SetRange(xax->GetXmin(),xax->GetXmax());
    if (hDump){
      hfit->GetListOfFunctions()->AddLast(&fgaus);
      (*pcstream)<<TString::Format("%sDump", tname).Data()<<
                 "entries="<<nrm<<     // number of entries
                 "isFitValid="<<isFitValid<< // true if the gaus fit converged
                 "hDump.="<<hDump<<    // histogram  - by default not filled
                 "mean0="<<mean0<<       // mean value of the last dimension - without fraction cut
                 "rms0="<<rms0<<         // rms value of the last dimension - without fraction cut
                 "mean="<<mean<<       // mean value of the last dimension
                 "rms="<<rms<<         // rms value of the last dimension
                 "m3="<<m3<<            // m3 (skewnes) of the last dimension
                 "m4="<<m4<<            // m4 (kurtosis) of the last dimension
                 "binMedian="<<binMedian<< //binned median value of 1D histogram
                 "entriesG="<<entriesG<<
                 "ndf="<<ndf<<          // number of degreed of freedom
                 "meanG="<<meanG<<     // mean of the gaus fit
                 "rmsG="<<rmsG<<       // rms of the gaus fit
                 "vecLTM.="<<vecLTM[0]<<   // LTM  frac% statistic
                 "chi2G="<<chi2G;      // chi2 of the gaus fit
      for (Int_t iest=1; iest<nestimators; iest++)
        (*pcstream)<<TString::Format("%sDump", tname).Data()<<TString::Format("vecLTM%d.=",iest)<<vecLTM[iest];   // LTM  frac% statistic

    }
      (*pcstream)<<tname<<
               "entries="<<nrm<<     // number of entries
               "isFitValid="<<isFitValid<< // true if the gaus fit converged
               "mean0="<<mean0<<       // mean value of the last dimension - without fraction cut
               "rms0="<<rms0<<         // rms value of the last dimension - without fraction cut
               "mean="<<mean<<       // mean value of the last dimension
               "rms="<<rms<<         // rms value of the last dimension
               "m3="<<m3<<            // m3 (skewnes) of the last dimension
               "m4="<<m4<<            // m4 (kurtosis) of the last dimension
               "binMedian="<<binMedian<< //binned median value of 1D histogram
               "entriesG="<<entriesG<<   //
               "meanG="<<meanG<<     // mean of the gaus fit
               "rmsG="<<rmsG<<       // rms of the gaus fit
               "vecLTM.="<<vecLTM[0]<<   // LTM  frac% statistic
               "ndf="<<ndf<<          // number of degrees of freedom
               "chi2G="<<chi2G;      // chi2 of the gaus fit

    if (exportGraph){
      /// TODO - make function to adapt the content without new
      graphHisto=new TGraphErrors(hfit);
      (*pcstream)<<tname<<"gr.="<<graphHisto;
    }
    if (exportGraphCumulative){
      /// TODO - make function to adapt the content without new
      TH1* hisCumulative=hfit->GetCumulative();
      graphCumulative = new TGraphErrors(hisCumulative);
      delete hisCumulative;
      (*pcstream)<<tname<<"grCumul.="<<graphCumulative;
    }

    for (Int_t iest=1; iest<nestimators; iest++)
      (*pcstream)<<tname<<TString::Format("vecLTM%d.=",iest)<<vecLTM[iest];   // LTM  frac% statistic


    //
    meanVector[tgtDim] = mean; // what's a point of this?
    for (Int_t idim=0; idim<ndim; idim++){
      snprintf(aname, 100, "%sMean=",histo->GetAxis(idim)->GetName());
      (*pcstream)<<tname<<
                 aname<<meanVectorMI[idim];      // current bin means
    }
    //
    for (Int_t iIter=0; iIter<ndim; iIter++){
      Int_t idim = axOrd[iIter];
      binVector[idim] = idx[idim];
      centerVector[idim] = histo->GetAxis(idim)->GetBinCenter(idx[idim]);
      snprintf(bname, 100, "%sBin=",histo->GetAxis(idim)->GetName());
      snprintf(cname, 100, "%sCenter=",histo->GetAxis(idim)->GetName());
      (*pcstream)<<tname<<
                 bname<<binVector[idim]<<      // current bin values
                 cname<<centerVector[idim];    // current bin centers
      if (hDump){
        (*pcstream)<<TString::Format("%sDump", tname).Data()<<
                   bname<<binVector[idim]<<      // current bin values
                   cname<<centerVector[idim];    // current bin centers
      }
    }
    (*pcstream)<<tname<<"\n";
    delete graphHisto;
    delete graphCumulative;
    graphHisto=NULL;
    graphCumulative=NULL;
    if (hDump)	{
      (*pcstream)<<TString::Format("%sDump", tname).Data()<<"\n";
      hfit->GetListOfFunctions()->RemoveLast();
    }
    // << ------------- do fit
    //
    if ( fitProgress>0 && nfits>0) {
      if   (((++fitCount)%fitProgress)==0) {
        printf("fit %lld %4.1f%% done\n",fitCount,100*double(fitCount)/nfits);
      }
    }
    //
    //next global bin in which target dimention will be looped
    for (dimVar=1;dimVar<ndim;dimVar++) { // varying dimension
      dimVarID = axOrd[dimVar]; // real axis id in the histo
      if ( (idx[dimVarID]+=binSt[dimVar]) > nbins[dimVarID] ) idx[dimVarID]=1;
      else break;
    }
    if (dimVar==ndim) break;
  }
  delete hfit;
  sw.Stop();
  sw.Print();
  for (Int_t iest=0; iest<nestimators; iest++) {
    delete vecLTM[iest];
  }

  /*
  int nb = histo->GetNbins();
  int prc = nb/100;
  for (int i=0;i<nb;i++) {
    histo->GetBinContent(i);
    if (i && (i%prc)==0) printf("Done %d%%\n",int(float(100*i)/prc));
  }
  */
}


/// Remap bin labels for sparse graphs
/// \param graph0   - sparse graph to  be rebinned
/// \param graph1   - graph with "template x axis"   (default)
/// \param option   - TODO - implement merge option as an option
/*!
### Example - remap sparse run list of QA.EVS graph to be like as in Logbook run list
\code
  AliExternalInfo info; tree = info->GetTree("QA.TPC","LHC15o","pass1","Logbook;QA.TPC;QA.EVS")
  graph = TStatToolkit::MakeMultGraph(tree,"","Logbook.run;QA.EVS.run:run","run>0","21;21","2;4",1.0,1,4,0);
  graph1 = (TGraph*)graph->GetListOfGraphs()->At(0); graph0 = (TGraph*)graph->GetListOfGraphs()->At(1);
  //
  TStatToolkit::RebinSparseGraph(graph0,graph1,"");
  TStatToolkit::DrawMultiGraph(graph,"ap");
\endcode
*/
void TStatToolkit::RebinSparseGraph(TGraph * graph0, TGraph *graph1, Option_t * option){
  if (graph0==NULL) throw std::invalid_argument( "RebinSparseGraph.graph0");
  if (graph1==NULL) throw std::invalid_argument( "RebinSparseGraph.graph1");
  TString opt = option;
  opt.ToLower();
  map<string,int> mapStrInt0, mapStrInt1;
  map<int,string> mapIntStr0, mapIntStr1;
  for (Int_t i=1; i<=graph0->GetXaxis()->GetNbins(); i++){
    mapStrInt0[graph0->GetXaxis()->GetBinLabel(i)]=i;
    mapIntStr0[i]=graph0->GetXaxis()->GetBinLabel(i);
  }
  for (Int_t i=1; i<=graph1->GetXaxis()->GetNbins(); i++){
    mapStrInt1[graph1->GetXaxis()->GetBinLabel(i)]=i;
    mapIntStr1[i]=graph1->GetXaxis()->GetBinLabel(i);
  }
  if (opt.Contains("merge")) {
    ::Error("Not supported option","%s",opt.Data());
  }
  for (Int_t i=1; i<=graph0->GetXaxis()->GetNbins(); i++){
    Int_t indexNew=mapStrInt1[mapIntStr0[i]];
    Double_t offset=graph0->GetX()[i-1]-int(graph0->GetX()[i-1]);
    graph0->GetX()[i-1]=indexNew+offset-1;
  }
  graph0->GetXaxis()->Set(graph1->GetXaxis()->GetNbins(),graph1->GetXaxis()->GetXbins()->GetArray());
  for (Int_t i=1; i<=graph0->GetXaxis()->GetNbins(); ++i){
    graph0->GetXaxis()->SetBinLabel(i,graph1->GetXaxis()->GetBinLabel(i));
  }
}

/// Rebin sparse MultiGraph to have the same granularity of x axis as in reference graphRef
/// \param multiGraph   - multiGraph to remap
/// \param graphRef     - reference graph
///
/// Used e.g to remap graphs in case of missing measurement in one of the sources
///  * e.g  Logbook run list for period LHC15o  is longer than run list for the QA.EVS and QA.TPC  "pass3_lowIR_pidfix"
/*!
\code
  AliExternalInfo info; tree = info->GetTree("Logbook","LHC15o","pass3_lowIR_pidfix","Logbook;QA.TPC;QA.EVS")
  graph = TStatToolkit::MakeMultGraph(tree,"","Logbook.run;QA.TPC.run;QA.EVS.run:run","runDuration>600","25;21;24","1;2;4",1.5,1,4,0);
  TStatToolkit::RebinSparseMultiGraph(graph,NULL);
  TStatToolkit::DrawMultiGraph(graph,"ap");
\endcode
*/
void TStatToolkit::RebinSparseMultiGraph(TMultiGraph *multiGraph, TGraph *graphRef){
  if (multiGraph==NULL)            throw std::invalid_argument( "RebinSparseMultyGraph.multiGraph");
  if (multiGraph->GetListOfGraphs()==NULL) throw std::invalid_argument( "RebinSparseMultyGraph.multiGraph empty");
  if (graphRef==NULL) {
    graphRef=(TGraph*)multiGraph->GetListOfGraphs()->At(0);
  }
  for (Int_t i=0; i<multiGraph->GetListOfGraphs()->GetEntries(); i++){
    try {
      RebinSparseGraph((TGraph*)multiGraph->GetListOfGraphs()->At(i),graphRef);
    }catch(const std::invalid_argument& error){
      ::Error("RebinSparseMultiGraph","%s",error.what());
    }
  }
}

/// MakeMultiGraphSparse  - transform TMultiGraph  using sparse representation for X axis
/// * Currently the code is working only for the integer x values
/// * TODO Support for the string x values
/// \param multiGraph
///
///Example case: Rebin run lists
///* Input graph with run list for the logbook, QA.TPC and QA.EVS
///  * Logbook runs list 192 entries
///  * QA.TPC run list has 11 runs
///  * QA.EVS has 8 runs
///* Output multi graph
///  * x axis with 192 bins (superset of all runs)
/*!
\code
    AliExternalInfo info; tree = info->GetTree("Logbook","LHC15o","pass3_lowIR_pidfix","Logbook;QA.TPC;QA.EVS")
    graph = TStatToolkit::MakeMultGraph(tree,"","Logbook.run;QA.EVS.run;QA.TPC.run:run","runDuration>600","25;21;24","1;2;4",0,1,10,0);
    TStatToolkit::MakeMultiGraphSparse(graph);
    TStatToolkit::DrawMultiGraph(graph,"ap");
\endcode
*/
void TStatToolkit::MakeMultiGraphSparse(TMultiGraph *multiGraph) {
  //map<string,int> mapStrInt0, mapStrInt1;
  if (multiGraph == NULL) throw std::invalid_argument("MakeSparseMultiGraphInt.multiGraph");
  map<int, int> intCounter;
  map<int, int> intMap;
  vector<int> valueArray;
  const TList *grArray = multiGraph->GetListOfGraphs();
  for (Int_t iGr = 0; iGr < grArray->GetEntries(); ++iGr) {
    TGraph *iGraph = (TGraph *) grArray->At(iGr);
    for (Int_t iPoint = 0; iPoint < iGraph->GetN(); ++iPoint) {
      Int_t value = TMath::Nint(iGraph->GetX()[iPoint]);
      intCounter[value]++;
    }
  }
  for (std::map<int, int>::iterator iterator = intCounter.begin(); iterator != intCounter.end(); iterator++)
    valueArray.push_back(iterator->first);
  std::sort(valueArray.begin(), valueArray.begin());
  //stdsort(valueArray);
  for (UInt_t iValue = 0; iValue < valueArray.size(); iValue++) intMap[valueArray[iValue]] = iValue;
  //
  for (Int_t iGr = 0; iGr < grArray->GetEntries(); ++iGr) {
    TGraph *iGraph = (TGraph *) grArray->At(iGr);
    iGraph->GetXaxis()->Set(valueArray.size(), 0, valueArray.size());
    for (UInt_t iValue = 0; iValue < valueArray.size(); iValue++)
      iGraph->GetXaxis()->SetBinLabel(iValue + 1, TString::Format("%d", valueArray[iValue]).Data());
    for (Int_t iPoint = 0; iPoint < iGraph->GetN(); ++iPoint) {
      iGraph->GetX()[iPoint] = intMap[TMath::Nint(iGraph->GetX()[iPoint]) + 0.5];
    }
  }
  if (multiGraph->GetXaxis()) {
    multiGraph->GetXaxis()->Set(valueArray.size(), 0, valueArray.size());
    for (UInt_t iValue = 0; iValue < valueArray.size(); iValue++)
      multiGraph->GetXaxis()->SetBinLabel(iValue + 1, TString::Format("%d", valueArray[iValue]).Data());
  }
}


/// Adapt style for histogram created by TTree queries
/// \param tree          - input tree (owner of metadata)
/// \param histogram     - pointer to histogram to AdaptStyle
///                       if NULL tree->GetHistogram used
/// \param option        - draw option
/// \return
Int_t TStatToolkit::AdaptHistoMetadata(TTree* tree, TH1 *histogram, TString option){
  if (histogram==NULL) histogram= tree->GetHistogram();
  if (histogram==NULL) return -1;
  TNamed *named=0;
  named = TStatToolkit::GetMetadata(tree,TString(histogram->GetXaxis()->GetTitle())+".AxisTitle");
  if (named) histogram->GetXaxis()->SetTitle(named->GetTitle());
  named = TStatToolkit::GetMetadata(tree,TString(histogram->GetYaxis()->GetTitle())+".AxisTitle");
  if (named) histogram->GetYaxis()->SetTitle(named->GetTitle());
  if (histogram->GetZaxis()){
    named = TStatToolkit::GetMetadata(tree,TString(histogram->GetZaxis()->GetTitle())+".AxisTitle");
    if (named) histogram->GetZaxis()->SetTitle(named->GetTitle());
  }
  /// TODO - TEMPORARY -make z tilte vissible  - using pallet size  should be done using CSS
  TPaletteAxis *palette = (TPaletteAxis*)histogram->GetListOfFunctions()->FindObject("palette");
  if (palette) palette->SetX2NDC(0.92);
  histogram->Draw(option.Data());
  return 1;
}

THashList * TStatToolkit::GetMetadata(TTree *tree) {
  TList *list=(TList*)(tree->GetUserInfo());
  return (THashList*)((list!=nullptr)? list->FindObject("metaTable"):0);
}