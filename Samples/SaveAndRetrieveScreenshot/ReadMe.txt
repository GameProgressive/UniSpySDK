**SaveAndRetrieveScreenshot**

Summary: This sample uploads a screenshot (gamespylogo.bmp, in the project directory) to the Sake File Server, then stores the returned FileID in a Sake record along with some other associated metadata. We then go through the process of retrieving the file assuming we're starting fresh, which involves retrieving our own metadata record in order to get the FileID and then downloading the file.

Note that for this sample we created a Sake table (via the Sake Admin site) which we called "ScreenShots" - this is a Profile owned table with a Limit Per Owner set to 1, meaning our sample user can only store one record. This is the table we used to store the file metadata.  

Command line inputs: None.

Dependencies: This app runs standalone.
 
Custom Preprocessor Defines:
  SAKE_SAMPLE - ensures the Sake related common helper functions get defined   

