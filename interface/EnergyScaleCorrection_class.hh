#ifndef EgammaAnalysis_ElectronTools_EnergyScaleCorrection_class_h
#define EgammaAnalysis_ElectronTools_EnergyScaleCorrection_class_h
/// Read and get energy scale and smearings from .dat files
/**\class EnergyScaleCorrection_class EnergyScaleCorrection_class.cc Calibration/ZFitter/src/EnergyScaleCorrection_class.cc
 *  \author Shervin Nourbakhsh
 *
 */

/** Description
	This module is taken from the ECALELF package, used to derive the energy scales and smearings.

	There are two sub-classes:
	 - correctionValue_class that defines the corrections
     - correctionCategory_class that defines the categories
	 There is one map that associates the correction to the category (values read from text file)

	 There is one class that reads the text files with the corrections and returns the scale/smearings given the electron/photon properties
 */

#include <TString.h>
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <TRandom3.h>
#include <string>
#include <bitset>
#include <iomanip>
//============================== First auxiliary class
class correctionValue_class
{
public:
	// values
	float scale, scale_err, scale_err_syst, scale_err_gain;
	float rho, rho_err;
	float phi, phi_err;
	float Emean, Emean_err;

	correctionValue_class(void)
	{
		scale = 1;
		scale_err = 0;
		scale_err_syst = 0;
		scale_err_gain = 0;
		rho = 0;
		rho_err = 0;
		phi = 0;
		phi_err = 0;
		Emean = 0;
		Emean_err = 0;
	};

	// only multiply scale by now
	const correctionValue_class operator* (const correctionValue_class &a) const{
		correctionValue_class out;
		out.scale = a.scale * scale;
		out.scale_err = sqrt(a.scale_err*a.scale_err + scale_err*scale_err);
		
		return out;
	}

	friend std::ostream& operator << (std::ostream& os, const correctionValue_class a)
	{
		os <<  std::fixed 
		   << a.scale << " " << a.scale_err << " " << a.scale_err_syst << " " << a.scale_err_gain ;
		   // <<  "\t"
		   // << a.rho << " +/- " << a.rho_err
		   // <<  "\t"
		   // << a.phi << " +/- " << a.phi_err
		   // <<  "\t"
		   // << a.Emean << " +/- " << a.Emean_err;
		return os;
	};

	// friend std::ostream& operator << (std::ostream& os, const correctionValue_class a)
	// {
	// 	os <<  "( " << std::fixed 
	// 	   << a.scale << " +/- " << a.scale_err << " +/- " << a.scale_err_syst << " +/- " << a.scale_err_gain << ")"
	// 	   <<  "\t"
	// 	   << a.rho << " +/- " << a.rho_err
	// 	   <<  "\t"
	// 	   << a.phi << " +/- " << a.phi_err
	// 	   <<  "\t"
	// 	   << a.Emean << " +/- " << a.Emean_err;
	// 	return os;
	// };

};

//============================== Second auxiliary class
class correctionCategory_class
{
	// for class definition and ordering
public:
	unsigned int runmin;
	unsigned int runmax;
	float etamin; ///< min eta value for the bin
	float etamax; ///< max eta value for the bin
private:
	// definition of the variables used for binning and the min-max ranges

	float r9min;  ///< min R9 vaule for the bin
	float r9max;  ///< max R9 value for the bin
	float etmin;  ///< min Et value for the bin
	float etmax;  ///< max Et value for the bin
	unsigned int gain; ///< 12, 6, 1, 61 (double gain switch)


public:
	/** there are two constructors:
	    - the first using the values taken from the e/gamma object
	    - the second used to define the categories and the correction values
	*/

	/** This constructor uses a string to define the category
	    The string is used in the .dat file where the corrections are defined
	    The syntax of the strings follows the definitions in the ECALELF ElectronCategory_class: http://ecalelfs.github.io/ECALELF/d5/d11/classElectronCategory__class.html
	*/
	correctionCategory_class(TString category_); ///< constructor with name of the category according to ElectronCategory_class

	/// this constructor is used to assign a category to the electron/photon given values in input
	inline  correctionCategory_class(const unsigned int runNumber, const float etaEle, const float R9Ele, const float EtEle, const unsigned int gainSeed)
	{
		runmin = runNumber;
		runmax = runNumber;
		etamin = fabs(etaEle);
		etamax = fabs(etaEle);
		r9min = R9Ele;
		r9max = R9Ele;
		etmin = EtEle;
		etmax = EtEle;
		gain  = gainSeed;
	}

	inline correctionCategory_class(const unsigned int runMin, const unsigned int runMax,
									const float etaMin, const float etaMax,
									const float R9Min, const float R9Max,
									const float EtMin, const float EtMax,
									const unsigned int gainSeed):
		runmin(runMin), runmax(runMax),
		etamin(fabs(etaMin)), etamax(etaMax),
		r9min(R9Min), r9max(R9Max),
		etmin(EtMin), etmax(EtMax),
		gain(gainSeed){
	}

	inline correctionCategory_class intersection(const correctionCategory_class &a) const {
		correctionCategory_class out(a);
		out.runmin = std::max(a.runmin, runmin);
		out.etamin = std::max(a.etamin, etamin);
		out.r9min = std::max(a.r9min, r9min);
		out.etmin = std::max(a.etmin, etmin);
		out.runmax = std::min(a.runmax, runmax);
		out.etamax = std::min(a.etamax, etamax);
		out.r9max = std::min(a.r9max, r9max);
		out.etmax = std::min(a.etmax, etmax);
		//out.gain = a.gain; // to be checked, they should have the same gain....
		return out;
	}
	/// for ordering of the categories
	bool operator<(const correctionCategory_class& b) const;

	/// for DEBUG
	// friend std::ostream& operator << (std::ostream& os, const correctionCategory_class a)
	// {
	// 	os <<         "RUN("  << a.runmin << ", " << a.runmax << ")"
	// 	   << "\t" << "eta("  << a.etamin << ", " << a.etamax << ")"
	// 	   << "\t" << "r9("   << a.r9min  << ", " << a.r9max  << ")"
	// 	   << "\t" << "Et("   << a.etmin  << ", " << a.etmax  << ")"
	// 	   << "\t" << "gain(" << a.gain   << ")";

	// 	return os;
	// };
	friend std::ostream& operator << (std::ostream& os, const correctionCategory_class a)
	{
		os << a.runmin << " " << a.runmax 
		   << "\t" <<  a.etamin << " " << a.etamax 
		   << "\t" <<  a.r9min  << " " << a.r9max  
		   << "\t" <<  a.etmin  << " " << a.etmax  
		   << "\t" <<  a.gain;

		return os;
	};

};


//==============================
/// map associating the category and the correction
typedef std::map < correctionCategory_class, correctionValue_class > correction_map_t;



//============================== Main class
class EnergyScaleCorrection_class
{

public:
	enum fileFormat_t {
		UNKNOWN = 0,
		GLOBE,
		ECALELF_TOY,
		ECALELF,
		TABLE
	};

	enum paramSmear_t {
		kNone = 0,
		kRho,
		kPhi,
		kNParamSmear
	};

	enum scaleNuisances_t {
		scNone = 0,
		scStat,
		scSyst,
		scStatSyst,
		scGain,
		scStatGain,
		scAll
	};

public:
	EnergyScaleCorrection_class(std::string correctionFileName, unsigned int genSeed = 0);
	EnergyScaleCorrection_class() {}; ///< dummy constructor needed in ElectronEnergyCalibratorRun2
	~EnergyScaleCorrection_class(void);


//------------------------------ scales
	float ScaleCorrection(unsigned int runNumber, bool isEBEle, double R9Ele, double etaSCEle,
	                      double EtEle, unsigned int gainSeed = 0, std::bitset<scAll> uncBitMask = scNone ) const; ///< method to get energy scale corrections

	float ScaleCorrectionUncertainty(unsigned int runNumber, bool isEBEle,
	                                 double R9Ele, double etaSCEle, double EtEle, unsigned int gainSeed,
	                                 std::bitset<scAll> uncBitMask = scAll) const; ///< method to get scale correction uncertainties: it is:
	/**
	 * bit 0 = stat
	 * bit 1 = syst
	 * but 2 = gain
	 */

private:
	correctionValue_class getScaleCorrection(unsigned int runNumber, bool isEBEle, double R9Ele, double etaSCEle, double EtEle, unsigned int gainSeed) const; ///< returns the correction value class
	float getScaleOffset(unsigned int runNumber, bool isEBEle, double R9Ele, double etaSCEle, double EtEle, unsigned int gainSeed) const; // returns the correction value


	void ReadFromFile(TString filename); ///<   category  "runNumber"   runMin  runMax   deltaP  err_deltaP_per_bin err_deltaP_stat err_deltaP_syst

	// this method adds the correction values read from the txt file to the map
	void AddScale(TString category_, int runMin_, int runMax_, double deltaP_, double err_deltaP_, double err_syst_deltaP, double err_deltaP_gain);
	void AddScale(int runMin_, int runMax_, 
										   float etaMin, float etaMax,
										   float r9Min, float r9Max,
										   float etMin, float etMax,
										   unsigned int gain,
										   double deltaP_, double err_deltaP_, double err_syst_deltaP, double err_deltaP_gain);
	//============================== smearings
public:
	float getSmearingSigma(int runNumber, bool isEBEle, float R9Ele, float etaSCEle, float EtEle, unsigned int gainSeed, paramSmear_t par, float nSigma = 0.) const;
	float getSmearingSigma(int runNumber, bool isEBEle, float R9Ele, float etaSCEle, float EtEle, unsigned int gainSeed, float nSigma_rho, float nSigma_phi) const;


private:
	fileFormat_t smearingType_;

	correction_map_t scales, scales_not_defined;
	correction_map_t smearings, smearings_not_defined;

	void AddSmearing(TString category_, int runMin_, int runMax_, //double smearing_, double err_smearing_);
	                 double rho, double err_rho, double phi, double err_phi, double Emean, double err_Emean);
	void ReadSmearingFromFile(TString filename); ///< File structure: category constTerm alpha;
public:
	inline void SetSmearingType(fileFormat_t value)
	{
		if(value >= 0 && value <= 1) {
			smearingType_ = value;
		} else {
			smearingType_ = UNKNOWN;
		}
	};

	float getSmearingRho(int runNumber, bool isEBEle, float R9Ele, float etaSCEle, float EtEle, unsigned int gainSeed) const; ///< public for sigmaE estimate



};


#endif
