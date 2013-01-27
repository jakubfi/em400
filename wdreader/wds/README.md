Winchester Data Sampler is a software for Logic16 to read data from Winchester Disk.
It uses SaleaeDeviceSdk 1.1.14, so you'll need libSaleaeDevice.so and SaleaeDeviceApi.h

To make it all work you need to pass MFM READ+ and MFM READ- signals from the disk through
a TI SN75176BP (or similar, at least 10MHz capable RS422-TTL signal converter). WDS expects
INDEX signal on Channel 0 and MFM DATA signal on Channel 1.

