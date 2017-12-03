**StoreSavedGameAndRetrieve**

Summary: This sample uses Sake to create a record with some dummy data intended to emulate a saved game. Once this record is created, it then retrieves the record (using sakeGetMyrecords) and iterates through the data, printing it to the console. Note that to accomplish this in your game you'll want to use sakeUpdateRecord (rather than sakeCreateRecord) if the player's record already exists. You can run the Release configuration to avoid debug output and hence make it easier to follow along.

*Note: These samples are very simple implementations intended to be used as a basic reference to help you learn and implement the SDKs. As such they are not cross-platform and don't have Unicode configurations. For a more thorough conglomeration of cross-platform calls with Unicode support you can check out the test apps within each SDK directory.   
 
Command line inputs: None

Dependencies: This app runs standalone.
 
Custom Preprocessor Defines:
  SAKE_SAMPLE - ensures the Sake related common helper functions get defined   

 
 