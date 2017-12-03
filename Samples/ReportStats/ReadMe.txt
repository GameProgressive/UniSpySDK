**ReportStats**

Summary: This app generates some sample match data for multiple players (defaulting to 3) for a racing game, then reports it via the ATLAS SDK to aggregate the corresponding stats. Note that these stats can then be retrieved (also via the ATLAS SDK) using Queries defined on the ATLAS admin site (see QueryStats sample). 

For ease we have all players report from this same app, though for your game the players will report from their separate apps. And all players use 'Authoritative' reporting, meaning that majority-rule will decide which value to keep in the case of conflicting values in the match reports (for even - eg. 1 to 1 - discrepencies that key is thrown out). 

Reference http://docs.poweredbygamespy.com/wiki/ATLAS_-_Walking_through_a_Ruleset for an overview and walk-through of the ATLAS Ruleset used by this Sample.   

*Note: These samples are very simple implementations intended to be used as a basic reference to help you learn and implement the SDKs. As such they are not cross-platform and don't have Unicode configurations. For a more thorough conglomeration of cross-platform calls with Unicode support you can check out the test apps within each SDK directory.   
 
Command line inputs: None

Dependencies: This app runs standalone.
 
Custom Preprocessor Defines:
  ATLAS_SAMPLE - ensures the ATLAS related common helper functions get defined   

 
 