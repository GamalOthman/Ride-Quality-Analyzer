# Ride-Quality-Analyzer
Short Discription: Measuring the ride quality of a car using data from accelerometer app in Andriod.

This code uses data saved by the Android app "Accelerometer Analyzer". The app saves the data from the accelerometer in a .txt file.
Data contains readings from accelerometer in the three axis (x, y, z) and time between each sample taken.
Data is represented in the .txt file in four columns in the format:

X Y Z time_from_previous_sample(ms)

The code analyzes the raw data by making arrays from each of the columns x, y, z and t. 
Then it performs some operations like deleting the leading 5 seconds in the data and the last 5 seconds if the user wants to do that.

A report is generated indicating the levels of vibration. Level1 through level20 or more.
Indicating how many vibrations have exceeded each level and the percentage of that to the total vibrations.
level1 is the vibrations that exceed 1 m/sec^2. Level2 is the vibrations that exceed 2 m/sec^2 and so on.

Also, a ride quality rating is presented. It is a number of 10 stars that depends on some rules. 
You can observe such rules it in the code.

## Supported Android Apps

Till now, only this app is supported:

* Accelerometer Analyzer: http://tinyurl.com/y4swylfs

## Recording

### Accelerometer Analyzer app

Open the app and tap on this icon 💾, then tap on "Start Writing". 
The app will start saving a file with a name like "accel__1.txt". You can find this file in a directory called "accelerometerDir" mostly in your phone storage.

When recording, make sure the phone is not moving. The code will dismiss the first 5 seconds of recording and also the last 5 ones, so do not worry about that.

**Important note:** You must enable "Remove Earth Gravity" feature in the app settings in order for code to work properly. Further developments of the code will implement this feature.

## Notes

As every app generates different output. analyzeFile() function in the code need to be modified to to adapt to the data.
So I will add more supported apps in the near future.

