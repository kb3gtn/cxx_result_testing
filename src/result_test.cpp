///////////////////////////////////////////////////
// Simple program to test result return type
// 
// Peter Fetterer
///////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <stdexcept>
#include <iostream>

#include "result.hpp"

// error type for a SDR
struct SDRError {
    enum class ErrorType {
      // list of errors types that can be reported by this error object
      conversion_error,
      key_error,
      range_error,
    };

    SDRError(ErrorType et, std::string message=""):
      error_type(et), mesg(message) {}

    ErrorType error_type;
    std::string mesg;
};

using std::cout;
using std::endl;
using std::string;

// structrure to represent a parameter value
struct parameter {
  // what data type is respresented here..
  enum class dtype {
    string_type,
    double_type,
    bool_type
  };
  // for double values, specify min/max values
  // set both to zero to not check range
  double m_min, m_max;
  string m_value;
  dtype m_dt;

  parameter() {
    m_value = std::string("");
    m_min = 0;
    m_max = 0;
    m_dt = dtype::string_type;

  }

  // constructors
  parameter( std::string value, double min=0, double max=0, dtype dt=dtype::string_type ) {
    m_value = value;
    m_min = min;
    m_max = max;
    m_dt = dt;
  }

  parameter( double value, double min=0, double max=0, dtype dt=dtype::double_type ) {
    m_value = std::to_string(value);
    m_min = min;
    m_max = max;
    m_dt = dt;
  }
    
  parameter( bool value, dtype dt=dtype::bool_type) {
    m_dt = dt;
    if ( value ) {
      m_value = std::string("true");
    } else {
      m_value = std::string("false");
    }
  }

  result::Result<double, SDRError> as_double() {
    // in C++ stod() throws exceptions when it fails, so catch and return result<err> to indicate conversion failure.
    try { 
      double d = std::stod(m_value);
      return result::Ok(d);
    } catch ( std::invalid_argument const& ex ) {
      return result::Err(SDRError(SDRError::ErrorType::conversion_error, "double conversion error on value: "+m_value));
    }
  }

  result::Result<bool,SDRError> as_bool() {
    if (std::string("True").compare(m_value) == 0 ) {
      return result::Ok(true);
    }
    if (std::string("true").compare(m_value) == 0 ) {
      return result::Ok(true);
    }
    if (std::string("False").compare(m_value) == 0 ) {
      return result::Ok(false);
    }

    if (std::string("false").compare(m_value) == 0 ) {
      return result::Ok(false);
    }
    return result::Err(SDRError(SDRError::ErrorType::conversion_error, "bool conversion error on value: "+m_value));
  }

};

using KVP = std::pair<string,parameter>;
using KVP_MAP = std::map<string,parameter>;



// some class representing Software Defined Radio hardware
class DummySDR {

    KVP_MAP param_map;
    
    void setup_default_kvp() {
      param_map["ch0_frequency"] = parameter( 446500000.0, 70e6, 6e9);
      param_map["ch0_tx_bb_bw"] = parameter( 500000.0, 500000, 54e6);
      param_map["ch0_rx_bb_bw"] = parameter( 500000.0, 500000, 54e6);
      param_map["ch0_tx_sample_rate"] = parameter( 1e6, 1e6, 64e6);
      param_map["ch0_rx_sample_rate"] = parameter( 1e6, 1e6, 64e6);
      param_map["ch0_tx_enabled"] = parameter( false );
      param_map["ch0_rx_enabled"] = parameter( false );
      param_map["ch0_tx_agc_enabled"] = parameter( true );
      param_map["ch0_rx_agc_enabled"] = parameter( true );
      param_map["ch0_tx_gain"] = parameter( 0.0, -2.0, 60 );
      param_map["ch0_rx_gain"] = parameter( 0.0, -2.0, 60 );
      param_map["ch1_frequency"] = parameter( 446500000.0, 70e6, 6e9);
      param_map["ch1_tx_bb_bw"] = parameter( 500000.0, 500000, 54e6);
      param_map["ch1_rx_bb_bw"] = parameter( 500000.0, 500000, 54e6);
      param_map["ch1_tx_sample_rate"] = parameter( 1e6, 1e6, 64e6);
      param_map["ch1_rx_sample_rate"] = parameter( 1e6, 1e6, 64e6);
      param_map["ch1_tx_enabled"] = parameter( false );
      param_map["ch1_rx_enabled"] = parameter( false );
      param_map["ch1_tx_agc_enabled"] = parameter( true );
      param_map["ch1_rx_agc_enabled"] = parameter( true );
      param_map["ch1_tx_gain"] = parameter( 0.0, -2.0, 60 );
      param_map["ch1_rx_gain"] = parameter( 0.0, -2.0, 60 );
    }

  public:
    DummySDR() {
      setup_default_kvp();
    }

    //////////////////////////////////////////////
    // try to set the value of a parameter
    //////////////////////////////////////////////
    
    // set using a string
    result::Result<bool, SDRError> set_parameter( std::string key, std::string value ) {
      // check if key exists in dict.
      if ( param_map.find(key) == param_map.end() ) {
        // key not found, return key error result
        return result::Err(SDRError(SDRError::ErrorType::key_error, "unknown key: "+key));
      }
      if ( param_map[key].m_dt != parameter::dtype::string_type ) {
        // TODO: add string value checks and perform conversions.. 
        // but for now only supporting same type storage.
        return result::Err(SDRError(SDRError::ErrorType::conversion_error, "key datatype is not a string: "+key));
      }
      // update value for this key..
      param_map[key].m_value = value;
      return result::Ok(false);
    }

    // set using a double
    result::Result<bool, SDRError> set_parameter( std::string key, double v) {
      // check if key exists in dict.
      if ( param_map.find(key) == param_map.end() ) {
        // key not found, return key error result
        return result::Err(SDRError(SDRError::ErrorType::key_error, "unknown key: "+key));
      }
      if ( param_map[key].m_dt != parameter::dtype::double_type ) {
        // TODO: add string value checks and perform conversions.. 
        // but for now only supporting same type storage.
        return result::Err(SDRError(SDRError::ErrorType::conversion_error, "key datatype is not a double: "+key));
      }
      // check range
      if ((param_map[key].m_min != 0) && (param_map[key].m_max != 0)) {
        // do range check
        if ((v < param_map[key].m_min) || (v > param_map[key].m_max) ) {
          return result::Err(SDRError(SDRError::ErrorType::range_error, "value of "+std::to_string(v)+" is outside allowed value range for key"+key ));
        }
      }
      // update value for this key.. value is stored as a string
      param_map[key].m_value = std::to_string(v);
      return result::Ok(false);
    }
  
    // set using a bool
    result::Result<bool, SDRError> set_parameter( std::string key, bool v) {
      // check if key exists in dict.
      if ( param_map.find(key) == param_map.end() ) {
        // key not found, return key error result
        return result::Err(SDRError(SDRError::ErrorType::key_error, "unknown key: "+key));
      }
      if ( param_map[key].m_dt != parameter::dtype::bool_type ) {
        return result::Err(SDRError(SDRError::ErrorType::conversion_error, "key datatype is not a bool" ));
      }
      // update value for this key..
      if (v) { 
        param_map[key].m_value = std::string("true");
      } else {
        param_map[key].m_value = std::string("false");
      }
      return result::Ok(false);
    }
 
    // try to set the value of a parameter
    // returns a result object indicating success and result, or error and error object
    result::Result<std::string, SDRError> get_as_string_parameter( std::string key ) {
      // check if key exists in dict.
      if ( param_map.find(key) == param_map.end() ) {
        // key not found, return key error result
        return result::Err(SDRError(SDRError::ErrorType::key_error, "unknown key: "+key));
      }
      return result::Ok(param_map[key].m_value);
    }

    result::Result<double, SDRError> get_as_double_parameter( std::string key ) {
      // check if key exists in dict.
      if ( param_map.find(key) == param_map.end() ) {
        // key not found, return key error result
        return result::Err(SDRError(SDRError::ErrorType::key_error, "unknown key: "+key));
      }
      if ( param_map[key].m_dt != parameter::dtype::double_type ) {
        // TODO: add string value checks and perform conversions.. 
        // but for now only supporting same type storage.
        return result::Err(SDRError(SDRError::ErrorType::conversion_error, "key datatype is not a double: "+key));
      }
      return param_map[key].as_double();
    }

    result::Result<bool, SDRError> get_as_bool_parameter( std::string key ) {
      // check if key exists in dict.
      if ( param_map.find(key) == param_map.end() ) {
        // key not found, return key error result
        return result::Err(SDRError(SDRError::ErrorType::key_error, "unknown key: "+key));
      }
      if ( param_map[key].m_dt != parameter::dtype::double_type ) {
        // TODO: add string value checks and perform conversions.. 
        // but for now only supporting same type storage.
        return result::Err(SDRError(SDRError::ErrorType::conversion_error, "key datatype is not a double: "+key));
      }
      return param_map[key].as_bool() ;
    }

};

int main() {

  DummySDR mySDR;

  // get ch0 frequency parameter
  std::cout << "ch0_frequency -> ";
  auto res1 = mySDR.get_as_double_parameter("ch0_frequency");
  if ( res1.is_err() ) {
    std::cout << "Failed to get parameter for key \"ch0_frequency\"" << std::endl;
  } else {
    double d = res1.unwrap();
    std::cout << d << std::endl;
  }
  // change ch0 freqneucy parameter
  std::cout << "ch0_frequency <= 440000000 ";
  auto res2 = mySDR.set_parameter(std::string("ch0_frequency"), 440000000.0 );
  if ( res2.is_err() ) {
    std::cout << "Failed ot update value for key \"ch0_frequency\"" << std::endl;
    // Get Error
    auto err = res2.unwrap_err();
    std::cout << "Error Message: " << err.mesg << std::endl;
  } else {
    std::cout << "Ok.." << std::endl;
  }

  // read back and see if it updated correctly..
  // get ch0 frequency parameter
  std::cout << "ch0_frequency -> ";
  auto res3 = mySDR.get_as_double_parameter("ch0_frequency");
  if ( res3.is_err() ) {
    std::cout << "Failed to get parameter for key \"ch0_frequency\"" << std::endl;
  } else {
    double d = res3.unwrap();
    std::cout << d << std::endl;
  }

  // try to set ch0_frequency to a string..
  // change ch0 freqneucy parameter
  std::cout << "ch0_frequency <= Mooo  ";
  auto res4 = mySDR.set_parameter(std::string("ch0_frequency"), std::string("Mooo") );
  if ( res4.is_err() ) {
    std::cout << "Failed ot update value for key \"ch0_frequency\"" << std::endl;
    // Get Error
    auto err = res4.unwrap_err();
    std::cout << "   -- Error Message: " << err.mesg << std::endl;
  } else {
    std::cout << "Ok.." << std::endl;
  }

  return 0;
}
