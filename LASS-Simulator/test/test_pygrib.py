from mpl_toolkits.basemap import Basemap  # import Basemap matplotlib toolkit
import numpy as np
import matplotlib.pyplot as plt
import pygrib # import pygrib interface to grib_api

def grb_to_grid(grb_obj):
    """Takes a single grb object containing multiple
    levels. Assumes same time, pressure levels. Compiles to a cube"""
    n_levels = len(grb_obj)
    levels = np.array([grb_element['level'] for grb_element in grb_obj])
    indexes = np.argsort(levels)[::-1] # highest pressure first
    cube = np.zeros([n_levels, grb_obj[0].values.shape[0], grb_obj[1].values.shape[1]])
    for i in range(n_levels):
        cube[i,:,:] = grb_obj[indexes[i]].values
    cube_dict = {'data' : cube, 'units' : grb_obj[0]['units'],
                 'levels' : levels[indexes]}
    return cube_dict

grbs = pygrib.open('../include/unittest/M-A0060-000.grb2')
for grb in grbs[:4]:
    for key in grb.keys():
        try:
            print("%s=%s" %(key,grb[key]))
        except:
            pass
    #print(grb)
#print(grb.keys())
#print(grb.validDate)
#print(grb.latitudes)
    
grbs.rewind() # rewind the iterator
from datetime import datetime
date_valid = datetime(2016,11,1,12)
t2mens = []
for grb in grbs:
    if grb.validDate == date_valid and grb.parameterName == 'Temperature' and grb.level == 2: 
        t2mens.append(grb.values)
        #print(grb.values)
t2mens = np.array(t2mens)
print(t2mens.shape, t2mens.min(), t2mens.max())
lats, lons = grb.latlons()  # get the lats and lons for the grid.
print('min/max lat and lon',lats.min(), lats.max(), lons.min(), lons.max())

grid = grb_to_grid(grbs.select(name='U component of wind'))
#grid['data'][0][1][10] 
#grid['data'][0][360][720] #[level_idx][lat -90->90(0.5 each)][long 0-359.5(0.5 each)]


if(0):
    fig = plt.figure(figsize=(16,35))
    #m = Basemap(projection='lcc',lon_0=-74,lat_0=41,width=4.e6,height=4.e6)
    m = Basemap(projection='lcc',lon_0=122,lat_0=22,width=4.e6,height=4.e6)
    x,y = m(lons,lats)
    for nens in range(1,51):
        ax = plt.subplot(10,5,nens)
        m.drawcoastlines()
        cs = m.contourf(x,y,t2mens[nens],np.linspace(230,300,41),cmap=plt.cm.jet,extend='both')
        t = plt.title('ens member %s' % nens)
    
"""
1:Geopotential Height:gpm (instant):regular_ll:isobaricInhPa:level 1000.0 Pa:fcst time 0 hrs:from 201611011200
2:Temperature:K (instant):regular_ll:isobaricInhPa:level 1000.0 Pa:fcst time 0 hrs:from 201611011200
3:U component of wind:m s**-1 (instant):regular_ll:isobaricInhPa:level 1000.0 Pa:fcst time 0 hrs:from 201611011200
4:V component of wind:m s**-1 (instant):regular_ll:isobaricInhPa:level 1000.0 Pa:fcst time 0 hrs:from 201611011200
['parametersVersion', 'UseEcmfConventions', 'GRIBEX_boustrophedonic', 'hundred', 'globalDomain', 'GRIBEditionNumber', 'grib2divider', 'angularPrecision', 'missingValue', 'ieeeFloats', 'isHindcast', 'section0Length', 'identifier', 'discipline', 'editionNumber', 'totalLength', 'sectionNumber', 'section1Length', 'numberOfSection', 'centre', 'centreDescription', 'subCentre', 'tablesVersion', 'masterDir', 'localTablesVersion', 'significanceOfReferenceTime', 'year', 'month', 'day', 'hour', 'minute', 'second', 'dataDate', 'julianDay', 'dataTime', 'productionStatusOfProcessedData', 'typeOfProcessedData', 'selectStepTemplateInterval', 'selectStepTemplateInstant', 'stepType', 'setCalendarId', 'deleteCalendarId', 'sectionNumber', 'grib2LocalSectionPresent', 'sectionNumber', 'gridDescriptionSectionPresent', 'section3Length', 'numberOfSection', 'sourceOfGridDefinition', 'numberOfDataPoints', 'numberOfOctectsForNumberOfPoints', 'interpretationOfNumberOfPoints', 'PLPresent', 'gridDefinitionTemplateNumber', 'gridDefinitionDescription', 'shapeOfTheEarth', 'scaleFactorOfRadiusOfSphericalEarth', 'scaledValueOfRadiusOfSphericalEarth', 'scaleFactorOfEarthMajorAxis', 'scaledValueOfEarthMajorAxis', 'scaleFactorOfEarthMinorAxis', 'scaledValueOfEarthMinorAxis', 'radius', 'Ni', 'Nj', 'basicAngleOfTheInitialProductionDomain', 'mBasicAngle', 'angleMultiplier', 'mAngleMultiplier', 'subdivisionsOfBasicAngle', 'angleDivisor', 'latitudeOfFirstGridPoint', 'longitudeOfFirstGridPoint', 'resolutionAndComponentFlags', 'resolutionAndComponentFlags1', 'resolutionAndComponentFlags2', 'iDirectionIncrementGiven', 'jDirectionIncrementGiven', 'uvRelativeToGrid', 'resolutionAndComponentFlags6', 'resolutionAndComponentFlags7', 'resolutionAndComponentFlags8', 'ijDirectionIncrementGiven', 'latitudeOfLastGridPoint', 'longitudeOfLastGridPoint', 'iDirectionIncrement', 'jDirectionIncrement', 'scanningMode', 'iScansNegatively', 'jScansPositively', 'jPointsAreConsecutive', 'alternativeRowScanning', 'iScansPositively', 'scanningMode5', 'scanningMode6', 'scanningMode7', 'scanningMode8', 'g2grid', 'latitudeOfFirstGridPointInDegrees', 'longitudeOfFirstGridPointInDegrees', 'latitudeOfLastGridPointInDegrees', 'longitudeOfLastGridPointInDegrees', 'iDirectionIncrementInDegrees', 'jDirectionIncrementInDegrees', 'latLonValues', 'latitudes', 'longitudes', 'distinctLatitudes', 'distinctLongitudes', 'gridType', 'sectionNumber', 'section4Length', 'numberOfSection', 'NV', 'neitherPresent', 'productDefinitionTemplateNumber', 'genVertHeightCoords', 'parameterCategory', 'parameterNumber', 'parameterUnits', 'parameterName', 'typeOfGeneratingProcess', 'backgroundProcess', 'generatingProcessIdentifier', 'hoursAfterDataCutoff', 'minutesAfterDataCutoff', 'indicatorOfUnitOfTimeRange', 'stepUnits', 'forecastTime', 'startStep', 'endStep', 'stepRange', 'stepTypeInternal', 'validityDate', 'validityTime', 'typeOfFirstFixedSurface', 'unitsOfFirstFixedSurface', 'nameOfFirstFixedSurface', 'scaleFactorOfFirstFixedSurface', 'scaledValueOfFirstFixedSurface', 'typeOfSecondFixedSurface', 'unitsOfSecondFixedSurface', 'nameOfSecondFixedSurface', 'scaleFactorOfSecondFixedSurface', 'scaledValueOfSecondFixedSurface', 'pressureUnits', 'typeOfLevel', 'level', 'bottomLevel', 'topLevel', 'tempPressureUnits', 'paramIdECMF', 'paramId', 'shortNameECMF', 'shortName', 'unitsECMF', 'units', 'nameECMF', 'name', 'cfNameECMF', 'cfName', 'cfVarNameECMF', 'cfVarName', 'ifsParam', 'PVPresent', 'deletePV', 'lengthOfHeaders', 'sectionNumber', 'section5Length', 'numberOfSection', 'numberOfValues', 'dataRepresentationTemplateNumber', 'packingType', 'referenceValue', 'referenceValueError', 'binaryScaleFactor', 'decimalScaleFactor', 'bitsPerValue', 'typeOfOriginalFieldValues', 'sectionNumber', 'section6Length', 'numberOfSection', 'bitMapIndicator', 'bitmapPresent', 'sectionNumber', 'section7Length', 'numberOfSection', 'codedValues', 'values', 'packingError', 'unpackedError', 'maximum', 'minimum', 'average', 'numberOfMissing', 'standardDeviation', 'skewness', 'kurtosis', 'isConstant', 'changeDecimalPrecision', 'decimalPrecision', 'setBitsPerValue', 'getNumberOfValues', 'scaleValuesBy', 'offsetValuesBy', 'productType', 'section8Length', 'analDate', 'validDate']
"""


"""
parametersVersion=1
UseEcmfConventions=1
GRIBEX_boustrophedonic=0
hundred=[100 100]
globalDomain=g
GRIBEditionNumber=2
grib2divider=1000000
angularPrecision=1000000
missingValue=9999
ieeeFloats=1
isHindcast=0
section0Length=16
identifier=GRIB
discipline=0
editionNumber=2
totalLength=779939
sectionNumber=[1 3 3 4 5 6 7]
section1Length=21
numberOfSection=[1 3 4 5 6 7]
centre=0
centreDescription=Absent
subCentre=0
tablesVersion=1
masterDir=grib2/tables/[tablesVersion]
localTablesVersion=0
significanceOfReferenceTime=1
year=2016
month=11
day=1
hour=12
minute=0
second=0
dataDate=20161101
julianDay=2457694.0
dataTime=1200
productionStatusOfProcessedData=0
typeOfProcessedData=af
selectStepTemplateInterval=1
selectStepTemplateInstant=1
stepType=instant
setCalendarId=0
deleteCalendarId=0
sectionNumber=[1 3 3 4 5 6 7]
grib2LocalSectionPresent=0
sectionNumber=[1 3 3 4 5 6 7]
gridDescriptionSectionPresent=1
section3Length=72
numberOfSection=[1 3 4 5 6 7]
sourceOfGridDefinition=0
numberOfDataPoints=259920
numberOfOctectsForNumberOfPoints=0
interpretationOfNumberOfPoints=0
PLPresent=0
gridDefinitionTemplateNumber=0
gridDefinitionDescription=Latitude/longitude. Also called equidistant cylindrical, or Plate Carree
shapeOfTheEarth=6
scaleFactorOfRadiusOfSphericalEarth=0
scaledValueOfRadiusOfSphericalEarth=0
scaleFactorOfEarthMajorAxis=0
scaledValueOfEarthMajorAxis=0
scaleFactorOfEarthMinorAxis=0
scaledValueOfEarthMinorAxis=0
radius=6371229
Ni=720
Nj=361
basicAngleOfTheInitialProductionDomain=0
mBasicAngle=0
angleMultiplier=1
mAngleMultiplier=1000000
subdivisionsOfBasicAngle=0
angleDivisor=1000000
latitudeOfFirstGridPoint=-90000000
longitudeOfFirstGridPoint=0
resolutionAndComponentFlags=48
resolutionAndComponentFlags1=0
resolutionAndComponentFlags2=0
iDirectionIncrementGiven=1
jDirectionIncrementGiven=1
uvRelativeToGrid=0
resolutionAndComponentFlags6=0
resolutionAndComponentFlags7=0
resolutionAndComponentFlags8=0
ijDirectionIncrementGiven=1
latitudeOfLastGridPoint=90000000
longitudeOfLastGridPoint=359500000
iDirectionIncrement=500000
jDirectionIncrement=500000
scanningMode=64
iScansNegatively=0
jScansPositively=1
jPointsAreConsecutive=0
alternativeRowScanning=0
iScansPositively=1
scanningMode5=0
scanningMode6=0
scanningMode7=0
scanningMode8=0
g2grid=[ -90.     0.    90.   359.5    0.5    0.5]
latitudeOfFirstGridPointInDegrees=-90.0
longitudeOfFirstGridPointInDegrees=0.0
latitudeOfLastGridPointInDegrees=90.0
longitudeOfLastGridPointInDegrees=359.5
iDirectionIncrementInDegrees=0.5
jDirectionIncrementInDegrees=0.5
latLonValues=[   -90.         0.     29960.929 ...,     90.       359.5    30125.264]
latitudes=[-90. -90. -90. ...,  90.  90.  90.]
longitudes=[   0.     0.5    1.  ...,  358.5  359.   359.5]
distinctLatitudes=[-90.  -89.5 -89.  -88.5 -88.  -87.5 -87.  -86.5 -86.  -85.5 -85.  -84.5
 -84.  -83.5 -83.  -82.5 -82.  -81.5 -81.  -80.5 -80.  -79.5 -79.  -78.5
 -78.  -77.5 -77.  -76.5 -76.  -75.5 -75.  -74.5 -74.  -73.5 -73.  -72.5
 -72.  -71.5 -71.  -70.5 -70.  -69.5 -69.  -68.5 -68.  -67.5 -67.  -66.5
 -66.  -65.5 -65.  -64.5 -64.  -63.5 -63.  -62.5 -62.  -61.5 -61.  -60.5
 -60.  -59.5 -59.  -58.5 -58.  -57.5 -57.  -56.5 -56.  -55.5 -55.  -54.5
 -54.  -53.5 -53.  -52.5 -52.  -51.5 -51.  -50.5 -50.  -49.5 -49.  -48.5
 -48.  -47.5 -47.  -46.5 -46.  -45.5 -45.  -44.5 -44.  -43.5 -43.  -42.5
 -42.  -41.5 -41.  -40.5 -40.  -39.5 -39.  -38.5 -38.  -37.5 -37.  -36.5
 -36.  -35.5 -35.  -34.5 -34.  -33.5 -33.  -32.5 -32.  -31.5 -31.  -30.5
 -30.  -29.5 -29.  -28.5 -28.  -27.5 -27.  -26.5 -26.  -25.5 -25.  -24.5
 -24.  -23.5 -23.  -22.5 -22.  -21.5 -21.  -20.5 -20.  -19.5 -19.  -18.5
 -18.  -17.5 -17.  -16.5 -16.  -15.5 -15.  -14.5 -14.  -13.5 -13.  -12.5
 -12.  -11.5 -11.  -10.5 -10.   -9.5  -9.   -8.5  -8.   -7.5  -7.   -6.5
  -6.   -5.5  -5.   -4.5  -4.   -3.5  -3.   -2.5  -2.   -1.5  -1.   -0.5
   0.    0.5   1.    1.5   2.    2.5   3.    3.5   4.    4.5   5.    5.5
   6.    6.5   7.    7.5   8.    8.5   9.    9.5  10.   10.5  11.   11.5
  12.   12.5  13.   13.5  14.   14.5  15.   15.5  16.   16.5  17.   17.5
  18.   18.5  19.   19.5  20.   20.5  21.   21.5  22.   22.5  23.   23.5
  24.   24.5  25.   25.5  26.   26.5  27.   27.5  28.   28.5  29.   29.5
  30.   30.5  31.   31.5  32.   32.5  33.   33.5  34.   34.5  35.   35.5
  36.   36.5  37.   37.5  38.   38.5  39.   39.5  40.   40.5  41.   41.5
  42.   42.5  43.   43.5  44.   44.5  45.   45.5  46.   46.5  47.   47.5
  48.   48.5  49.   49.5  50.   50.5  51.   51.5  52.   52.5  53.   53.5
  54.   54.5  55.   55.5  56.   56.5  57.   57.5  58.   58.5  59.   59.5
  60.   60.5  61.   61.5  62.   62.5  63.   63.5  64.   64.5  65.   65.5
  66.   66.5  67.   67.5  68.   68.5  69.   69.5  70.   70.5  71.   71.5
  72.   72.5  73.   73.5  74.   74.5  75.   75.5  76.   76.5  77.   77.5
  78.   78.5  79.   79.5  80.   80.5  81.   81.5  82.   82.5  83.   83.5
  84.   84.5  85.   85.5  86.   86.5  87.   87.5  88.   88.5  89.   89.5
  90. ]
distinctLongitudes=[   0.     0.5    1.     1.5    2.     2.5    3.     3.5    4.     4.5
    5.     5.5    6.     6.5    7.     7.5    8.     8.5    9.     9.5
   10.    10.5   11.    11.5   12.    12.5   13.    13.5   14.    14.5
   15.    15.5   16.    16.5   17.    17.5   18.    18.5   19.    19.5
   20.    20.5   21.    21.5   22.    22.5   23.    23.5   24.    24.5
   25.    25.5   26.    26.5   27.    27.5   28.    28.5   29.    29.5
   30.    30.5   31.    31.5   32.    32.5   33.    33.5   34.    34.5
   35.    35.5   36.    36.5   37.    37.5   38.    38.5   39.    39.5
   40.    40.5   41.    41.5   42.    42.5   43.    43.5   44.    44.5
   45.    45.5   46.    46.5   47.    47.5   48.    48.5   49.    49.5
   50.    50.5   51.    51.5   52.    52.5   53.    53.5   54.    54.5
   55.    55.5   56.    56.5   57.    57.5   58.    58.5   59.    59.5
   60.    60.5   61.    61.5   62.    62.5   63.    63.5   64.    64.5
   65.    65.5   66.    66.5   67.    67.5   68.    68.5   69.    69.5
   70.    70.5   71.    71.5   72.    72.5   73.    73.5   74.    74.5
   75.    75.5   76.    76.5   77.    77.5   78.    78.5   79.    79.5
   80.    80.5   81.    81.5   82.    82.5   83.    83.5   84.    84.5
   85.    85.5   86.    86.5   87.    87.5   88.    88.5   89.    89.5
   90.    90.5   91.    91.5   92.    92.5   93.    93.5   94.    94.5
   95.    95.5   96.    96.5   97.    97.5   98.    98.5   99.    99.5
  100.   100.5  101.   101.5  102.   102.5  103.   103.5  104.   104.5
  105.   105.5  106.   106.5  107.   107.5  108.   108.5  109.   109.5
  110.   110.5  111.   111.5  112.   112.5  113.   113.5  114.   114.5
  115.   115.5  116.   116.5  117.   117.5  118.   118.5  119.   119.5
  120.   120.5  121.   121.5  122.   122.5  123.   123.5  124.   124.5
  125.   125.5  126.   126.5  127.   127.5  128.   128.5  129.   129.5
  130.   130.5  131.   131.5  132.   132.5  133.   133.5  134.   134.5
  135.   135.5  136.   136.5  137.   137.5  138.   138.5  139.   139.5
  140.   140.5  141.   141.5  142.   142.5  143.   143.5  144.   144.5
  145.   145.5  146.   146.5  147.   147.5  148.   148.5  149.   149.5
  150.   150.5  151.   151.5  152.   152.5  153.   153.5  154.   154.5
  155.   155.5  156.   156.5  157.   157.5  158.   158.5  159.   159.5
  160.   160.5  161.   161.5  162.   162.5  163.   163.5  164.   164.5
  165.   165.5  166.   166.5  167.   167.5  168.   168.5  169.   169.5
  170.   170.5  171.   171.5  172.   172.5  173.   173.5  174.   174.5
  175.   175.5  176.   176.5  177.   177.5  178.   178.5  179.   179.5
  180.   180.5  181.   181.5  182.   182.5  183.   183.5  184.   184.5
  185.   185.5  186.   186.5  187.   187.5  188.   188.5  189.   189.5
  190.   190.5  191.   191.5  192.   192.5  193.   193.5  194.   194.5
  195.   195.5  196.   196.5  197.   197.5  198.   198.5  199.   199.5
  200.   200.5  201.   201.5  202.   202.5  203.   203.5  204.   204.5
  205.   205.5  206.   206.5  207.   207.5  208.   208.5  209.   209.5
  210.   210.5  211.   211.5  212.   212.5  213.   213.5  214.   214.5
  215.   215.5  216.   216.5  217.   217.5  218.   218.5  219.   219.5
  220.   220.5  221.   221.5  222.   222.5  223.   223.5  224.   224.5
  225.   225.5  226.   226.5  227.   227.5  228.   228.5  229.   229.5
  230.   230.5  231.   231.5  232.   232.5  233.   233.5  234.   234.5
  235.   235.5  236.   236.5  237.   237.5  238.   238.5  239.   239.5
  240.   240.5  241.   241.5  242.   242.5  243.   243.5  244.   244.5
  245.   245.5  246.   246.5  247.   247.5  248.   248.5  249.   249.5
  250.   250.5  251.   251.5  252.   252.5  253.   253.5  254.   254.5
  255.   255.5  256.   256.5  257.   257.5  258.   258.5  259.   259.5
  260.   260.5  261.   261.5  262.   262.5  263.   263.5  264.   264.5
  265.   265.5  266.   266.5  267.   267.5  268.   268.5  269.   269.5
  270.   270.5  271.   271.5  272.   272.5  273.   273.5  274.   274.5
  275.   275.5  276.   276.5  277.   277.5  278.   278.5  279.   279.5
  280.   280.5  281.   281.5  282.   282.5  283.   283.5  284.   284.5
  285.   285.5  286.   286.5  287.   287.5  288.   288.5  289.   289.5
  290.   290.5  291.   291.5  292.   292.5  293.   293.5  294.   294.5
  295.   295.5  296.   296.5  297.   297.5  298.   298.5  299.   299.5
  300.   300.5  301.   301.5  302.   302.5  303.   303.5  304.   304.5
  305.   305.5  306.   306.5  307.   307.5  308.   308.5  309.   309.5
  310.   310.5  311.   311.5  312.   312.5  313.   313.5  314.   314.5
  315.   315.5  316.   316.5  317.   317.5  318.   318.5  319.   319.5
  320.   320.5  321.   321.5  322.   322.5  323.   323.5  324.   324.5
  325.   325.5  326.   326.5  327.   327.5  328.   328.5  329.   329.5
  330.   330.5  331.   331.5  332.   332.5  333.   333.5  334.   334.5
  335.   335.5  336.   336.5  337.   337.5  338.   338.5  339.   339.5
  340.   340.5  341.   341.5  342.   342.5  343.   343.5  344.   344.5
  345.   345.5  346.   346.5  347.   347.5  348.   348.5  349.   349.5
  350.   350.5  351.   351.5  352.   352.5  353.   353.5  354.   354.5
  355.   355.5  356.   356.5  357.   357.5  358.   358.5  359.   359.5]
gridType=regular_ll
sectionNumber=[1 3 3 4 5 6 7]
section4Length=34
numberOfSection=[1 3 4 5 6 7]
NV=0
neitherPresent=0
productDefinitionTemplateNumber=0
genVertHeightCoords=0
parameterCategory=3
parameterNumber=5
parameterUnits=gpm
parameterName=Geopotential height
typeOfGeneratingProcess=2
backgroundProcess=0
generatingProcessIdentifier=0
hoursAfterDataCutoff=4294967295
minutesAfterDataCutoff=4294967295
indicatorOfUnitOfTimeRange=1
stepUnits=1
forecastTime=0
startStep=0
endStep=0
stepRange=0
stepTypeInternal=instant
validityDate=20161101
validityTime=1200
typeOfFirstFixedSurface=pl
unitsOfFirstFixedSurface=Pa
nameOfFirstFixedSurface=Isobaric surface
scaleFactorOfFirstFixedSurface=-2
scaledValueOfFirstFixedSurface=10
typeOfSecondFixedSurface=255
unitsOfSecondFixedSurface=unknown
nameOfSecondFixedSurface=Missing
scaleFactorOfSecondFixedSurface=0
scaledValueOfSecondFixedSurface=0
pressureUnits=hPa
typeOfLevel=isobaricInhPa
level=10
bottomLevel=10
topLevel=10
tempPressureUnits=hPa
paramIdECMF=156
paramId=156
shortNameECMF=gh
shortName=gh
unitsECMF=gpm
units=gpm
nameECMF=Geopotential Height
name=Geopotential Height
cfNameECMF=geopotential_height
cfName=geopotential_height
cfVarNameECMF=gh
cfVarName=gh
ifsParam=156
PVPresent=0
deletePV=1
lengthOfHeaders=127
sectionNumber=[1 3 3 4 5 6 7]
section5Length=21
numberOfSection=[1 3 4 5 6 7]
numberOfValues=259920
dataRepresentationTemplateNumber=0
packingType=grid_simple
referenceValue=29510388.0
referenceValueError=2.0
binaryScaleFactor=0
decimalScaleFactor=3
bitsPerValue=24
typeOfOriginalFieldValues=0
sectionNumber=[1 3 3 4 5 6 7]
section6Length=6
numberOfSection=[1 3 4 5 6 7]
bitMapIndicator=255
bitmapPresent=0
sectionNumber=[1 3 3 4 5 6 7]
section7Length=779765
numberOfSection=[1 3 4 5 6 7]
codedValues=[ 29960.929  29960.929  29960.929 ...,  30125.264  30125.264  30125.264]
values=[[ 29960.929  29960.929  29960.929 ...,  29960.929  29960.929  29960.929]
 [ 29943.079  29943.28   29943.483 ...,  29942.526  29942.71   29942.895]
 [ 29925.819  29926.212  29926.604 ...,  29924.731  29925.095  29925.458]
 ..., 
 [ 30105.005  30105.03   30105.055 ...,  30104.962  30104.975  30104.991]
 [ 30114.282  30114.307  30114.333 ...,  30114.235  30114.251  30114.266]
 [ 30125.264  30125.264  30125.264 ...,  30125.264  30125.264  30125.264]]
packingError=0.0015
unpackedError=2.0
maximum=31484.366
minimum=29510.388
average=30646.83025801727
numberOfMissing=0
standardDeviation=428.1459751017444
skewness=-0.763137296495686
kurtosis=-0.2788163412716229
isConstant=0.0
changeDecimalPrecision=3
decimalPrecision=3
setBitsPerValue=24
getNumberOfValues=259920
scaleValuesBy=1.0
offsetValuesBy=0.0
productType=unknown
section8Length=4
parametersVersion=1
UseEcmfConventions=1
GRIBEX_boustrophedonic=0
hundred=[100 100]
globalDomain=g
GRIBEditionNumber=2
grib2divider=1000000
angularPrecision=1000000
missingValue=9999
ieeeFloats=1
isHindcast=0
section0Length=16
identifier=GRIB
discipline=0
editionNumber=2
totalLength=779939
sectionNumber=[1 3 3 4 5 6 7]
section1Length=21
numberOfSection=[1 3 4 5 6 7]
centre=0
centreDescription=Absent
subCentre=0
tablesVersion=1
masterDir=grib2/tables/[tablesVersion]
localTablesVersion=0
significanceOfReferenceTime=1
year=2016
month=11
day=1
hour=12
minute=0
second=0
dataDate=20161101
julianDay=2457694.0
dataTime=1200
productionStatusOfProcessedData=0
typeOfProcessedData=af
selectStepTemplateInterval=1
selectStepTemplateInstant=1
stepType=instant
setCalendarId=0
deleteCalendarId=0
sectionNumber=[1 3 3 4 5 6 7]
grib2LocalSectionPresent=0
sectionNumber=[1 3 3 4 5 6 7]
gridDescriptionSectionPresent=1
section3Length=72
numberOfSection=[1 3 4 5 6 7]
sourceOfGridDefinition=0
numberOfDataPoints=259920
numberOfOctectsForNumberOfPoints=0
interpretationOfNumberOfPoints=0
PLPresent=0
gridDefinitionTemplateNumber=0
gridDefinitionDescription=Latitude/longitude. Also called equidistant cylindrical, or Plate Carree
shapeOfTheEarth=6
scaleFactorOfRadiusOfSphericalEarth=0
scaledValueOfRadiusOfSphericalEarth=0
scaleFactorOfEarthMajorAxis=0
scaledValueOfEarthMajorAxis=0
scaleFactorOfEarthMinorAxis=0
scaledValueOfEarthMinorAxis=0
radius=6371229
Ni=720
Nj=361
basicAngleOfTheInitialProductionDomain=0
mBasicAngle=0
angleMultiplier=1
mAngleMultiplier=1000000
subdivisionsOfBasicAngle=0
angleDivisor=1000000
latitudeOfFirstGridPoint=-90000000
longitudeOfFirstGridPoint=0
resolutionAndComponentFlags=48
resolutionAndComponentFlags1=0
resolutionAndComponentFlags2=0
iDirectionIncrementGiven=1
jDirectionIncrementGiven=1
uvRelativeToGrid=0
resolutionAndComponentFlags6=0
resolutionAndComponentFlags7=0
resolutionAndComponentFlags8=0
ijDirectionIncrementGiven=1
latitudeOfLastGridPoint=90000000
longitudeOfLastGridPoint=359500000
iDirectionIncrement=500000
jDirectionIncrement=500000
scanningMode=64
iScansNegatively=0
jScansPositively=1
jPointsAreConsecutive=0
alternativeRowScanning=0
iScansPositively=1
scanningMode5=0
scanningMode6=0
scanningMode7=0
scanningMode8=0
g2grid=[ -90.     0.    90.   359.5    0.5    0.5]
latitudeOfFirstGridPointInDegrees=-90.0
longitudeOfFirstGridPointInDegrees=0.0
latitudeOfLastGridPointInDegrees=90.0
longitudeOfLastGridPointInDegrees=359.5
iDirectionIncrementInDegrees=0.5
jDirectionIncrementInDegrees=0.5
latLonValues=[ -90.         0.       257.86242 ...,   90.       359.5      203.76511]
latitudes=[-90. -90. -90. ...,  90.  90.  90.]
longitudes=[   0.     0.5    1.  ...,  358.5  359.   359.5]
distinctLatitudes=[-90.  -89.5 -89.  -88.5 -88.  -87.5 -87.  -86.5 -86.  -85.5 -85.  -84.5
 -84.  -83.5 -83.  -82.5 -82.  -81.5 -81.  -80.5 -80.  -79.5 -79.  -78.5
 -78.  -77.5 -77.  -76.5 -76.  -75.5 -75.  -74.5 -74.  -73.5 -73.  -72.5
 -72.  -71.5 -71.  -70.5 -70.  -69.5 -69.  -68.5 -68.  -67.5 -67.  -66.5
 -66.  -65.5 -65.  -64.5 -64.  -63.5 -63.  -62.5 -62.  -61.5 -61.  -60.5
 -60.  -59.5 -59.  -58.5 -58.  -57.5 -57.  -56.5 -56.  -55.5 -55.  -54.5
 -54.  -53.5 -53.  -52.5 -52.  -51.5 -51.  -50.5 -50.  -49.5 -49.  -48.5
 -48.  -47.5 -47.  -46.5 -46.  -45.5 -45.  -44.5 -44.  -43.5 -43.  -42.5
 -42.  -41.5 -41.  -40.5 -40.  -39.5 -39.  -38.5 -38.  -37.5 -37.  -36.5
 -36.  -35.5 -35.  -34.5 -34.  -33.5 -33.  -32.5 -32.  -31.5 -31.  -30.5
 -30.  -29.5 -29.  -28.5 -28.  -27.5 -27.  -26.5 -26.  -25.5 -25.  -24.5
 -24.  -23.5 -23.  -22.5 -22.  -21.5 -21.  -20.5 -20.  -19.5 -19.  -18.5
 -18.  -17.5 -17.  -16.5 -16.  -15.5 -15.  -14.5 -14.  -13.5 -13.  -12.5
 -12.  -11.5 -11.  -10.5 -10.   -9.5  -9.   -8.5  -8.   -7.5  -7.   -6.5
  -6.   -5.5  -5.   -4.5  -4.   -3.5  -3.   -2.5  -2.   -1.5  -1.   -0.5
   0.    0.5   1.    1.5   2.    2.5   3.    3.5   4.    4.5   5.    5.5
   6.    6.5   7.    7.5   8.    8.5   9.    9.5  10.   10.5  11.   11.5
  12.   12.5  13.   13.5  14.   14.5  15.   15.5  16.   16.5  17.   17.5
  18.   18.5  19.   19.5  20.   20.5  21.   21.5  22.   22.5  23.   23.5
  24.   24.5  25.   25.5  26.   26.5  27.   27.5  28.   28.5  29.   29.5
  30.   30.5  31.   31.5  32.   32.5  33.   33.5  34.   34.5  35.   35.5
  36.   36.5  37.   37.5  38.   38.5  39.   39.5  40.   40.5  41.   41.5
  42.   42.5  43.   43.5  44.   44.5  45.   45.5  46.   46.5  47.   47.5
  48.   48.5  49.   49.5  50.   50.5  51.   51.5  52.   52.5  53.   53.5
  54.   54.5  55.   55.5  56.   56.5  57.   57.5  58.   58.5  59.   59.5
  60.   60.5  61.   61.5  62.   62.5  63.   63.5  64.   64.5  65.   65.5
  66.   66.5  67.   67.5  68.   68.5  69.   69.5  70.   70.5  71.   71.5
  72.   72.5  73.   73.5  74.   74.5  75.   75.5  76.   76.5  77.   77.5
  78.   78.5  79.   79.5  80.   80.5  81.   81.5  82.   82.5  83.   83.5
  84.   84.5  85.   85.5  86.   86.5  87.   87.5  88.   88.5  89.   89.5
  90. ]
distinctLongitudes=[   0.     0.5    1.     1.5    2.     2.5    3.     3.5    4.     4.5
    5.     5.5    6.     6.5    7.     7.5    8.     8.5    9.     9.5
   10.    10.5   11.    11.5   12.    12.5   13.    13.5   14.    14.5
   15.    15.5   16.    16.5   17.    17.5   18.    18.5   19.    19.5
   20.    20.5   21.    21.5   22.    22.5   23.    23.5   24.    24.5
   25.    25.5   26.    26.5   27.    27.5   28.    28.5   29.    29.5
   30.    30.5   31.    31.5   32.    32.5   33.    33.5   34.    34.5
   35.    35.5   36.    36.5   37.    37.5   38.    38.5   39.    39.5
   40.    40.5   41.    41.5   42.    42.5   43.    43.5   44.    44.5
   45.    45.5   46.    46.5   47.    47.5   48.    48.5   49.    49.5
   50.    50.5   51.    51.5   52.    52.5   53.    53.5   54.    54.5
   55.    55.5   56.    56.5   57.    57.5   58.    58.5   59.    59.5
   60.    60.5   61.    61.5   62.    62.5   63.    63.5   64.    64.5
   65.    65.5   66.    66.5   67.    67.5   68.    68.5   69.    69.5
   70.    70.5   71.    71.5   72.    72.5   73.    73.5   74.    74.5
   75.    75.5   76.    76.5   77.    77.5   78.    78.5   79.    79.5
   80.    80.5   81.    81.5   82.    82.5   83.    83.5   84.    84.5
   85.    85.5   86.    86.5   87.    87.5   88.    88.5   89.    89.5
   90.    90.5   91.    91.5   92.    92.5   93.    93.5   94.    94.5
   95.    95.5   96.    96.5   97.    97.5   98.    98.5   99.    99.5
  100.   100.5  101.   101.5  102.   102.5  103.   103.5  104.   104.5
  105.   105.5  106.   106.5  107.   107.5  108.   108.5  109.   109.5
  110.   110.5  111.   111.5  112.   112.5  113.   113.5  114.   114.5
  115.   115.5  116.   116.5  117.   117.5  118.   118.5  119.   119.5
  120.   120.5  121.   121.5  122.   122.5  123.   123.5  124.   124.5
  125.   125.5  126.   126.5  127.   127.5  128.   128.5  129.   129.5
  130.   130.5  131.   131.5  132.   132.5  133.   133.5  134.   134.5
  135.   135.5  136.   136.5  137.   137.5  138.   138.5  139.   139.5
  140.   140.5  141.   141.5  142.   142.5  143.   143.5  144.   144.5
  145.   145.5  146.   146.5  147.   147.5  148.   148.5  149.   149.5
  150.   150.5  151.   151.5  152.   152.5  153.   153.5  154.   154.5
  155.   155.5  156.   156.5  157.   157.5  158.   158.5  159.   159.5
  160.   160.5  161.   161.5  162.   162.5  163.   163.5  164.   164.5
  165.   165.5  166.   166.5  167.   167.5  168.   168.5  169.   169.5
  170.   170.5  171.   171.5  172.   172.5  173.   173.5  174.   174.5
  175.   175.5  176.   176.5  177.   177.5  178.   178.5  179.   179.5
  180.   180.5  181.   181.5  182.   182.5  183.   183.5  184.   184.5
  185.   185.5  186.   186.5  187.   187.5  188.   188.5  189.   189.5
  190.   190.5  191.   191.5  192.   192.5  193.   193.5  194.   194.5
  195.   195.5  196.   196.5  197.   197.5  198.   198.5  199.   199.5
  200.   200.5  201.   201.5  202.   202.5  203.   203.5  204.   204.5
  205.   205.5  206.   206.5  207.   207.5  208.   208.5  209.   209.5
  210.   210.5  211.   211.5  212.   212.5  213.   213.5  214.   214.5
  215.   215.5  216.   216.5  217.   217.5  218.   218.5  219.   219.5
  220.   220.5  221.   221.5  222.   222.5  223.   223.5  224.   224.5
  225.   225.5  226.   226.5  227.   227.5  228.   228.5  229.   229.5
  230.   230.5  231.   231.5  232.   232.5  233.   233.5  234.   234.5
  235.   235.5  236.   236.5  237.   237.5  238.   238.5  239.   239.5
  240.   240.5  241.   241.5  242.   242.5  243.   243.5  244.   244.5
  245.   245.5  246.   246.5  247.   247.5  248.   248.5  249.   249.5
  250.   250.5  251.   251.5  252.   252.5  253.   253.5  254.   254.5
  255.   255.5  256.   256.5  257.   257.5  258.   258.5  259.   259.5
  260.   260.5  261.   261.5  262.   262.5  263.   263.5  264.   264.5
  265.   265.5  266.   266.5  267.   267.5  268.   268.5  269.   269.5
  270.   270.5  271.   271.5  272.   272.5  273.   273.5  274.   274.5
  275.   275.5  276.   276.5  277.   277.5  278.   278.5  279.   279.5
  280.   280.5  281.   281.5  282.   282.5  283.   283.5  284.   284.5
  285.   285.5  286.   286.5  287.   287.5  288.   288.5  289.   289.5
  290.   290.5  291.   291.5  292.   292.5  293.   293.5  294.   294.5
  295.   295.5  296.   296.5  297.   297.5  298.   298.5  299.   299.5
  300.   300.5  301.   301.5  302.   302.5  303.   303.5  304.   304.5
  305.   305.5  306.   306.5  307.   307.5  308.   308.5  309.   309.5
  310.   310.5  311.   311.5  312.   312.5  313.   313.5  314.   314.5
  315.   315.5  316.   316.5  317.   317.5  318.   318.5  319.   319.5
  320.   320.5  321.   321.5  322.   322.5  323.   323.5  324.   324.5
  325.   325.5  326.   326.5  327.   327.5  328.   328.5  329.   329.5
  330.   330.5  331.   331.5  332.   332.5  333.   333.5  334.   334.5
  335.   335.5  336.   336.5  337.   337.5  338.   338.5  339.   339.5
  340.   340.5  341.   341.5  342.   342.5  343.   343.5  344.   344.5
  345.   345.5  346.   346.5  347.   347.5  348.   348.5  349.   349.5
  350.   350.5  351.   351.5  352.   352.5  353.   353.5  354.   354.5
  355.   355.5  356.   356.5  357.   357.5  358.   358.5  359.   359.5]
gridType=regular_ll
sectionNumber=[1 3 3 4 5 6 7]
section4Length=34
numberOfSection=[1 3 4 5 6 7]
NV=0
neitherPresent=0
productDefinitionTemplateNumber=0
genVertHeightCoords=0
parameterCategory=0
parameterNumber=0
parameterUnits=K
parameterName=Temperature
typeOfGeneratingProcess=2
backgroundProcess=0
generatingProcessIdentifier=0
hoursAfterDataCutoff=4294967295
minutesAfterDataCutoff=4294967295
indicatorOfUnitOfTimeRange=1
stepUnits=1
forecastTime=0
startStep=0
endStep=0
stepRange=0
stepTypeInternal=instant
validityDate=20161101
validityTime=1200
typeOfFirstFixedSurface=pl
unitsOfFirstFixedSurface=Pa
nameOfFirstFixedSurface=Isobaric surface
scaleFactorOfFirstFixedSurface=-2
scaledValueOfFirstFixedSurface=10
typeOfSecondFixedSurface=255
unitsOfSecondFixedSurface=unknown
nameOfSecondFixedSurface=Missing
scaleFactorOfSecondFixedSurface=0
scaledValueOfSecondFixedSurface=0
pressureUnits=hPa
typeOfLevel=isobaricInhPa
level=10
bottomLevel=10
topLevel=10
tempPressureUnits=hPa
paramIdECMF=130
paramId=130
shortNameECMF=t
shortName=t
unitsECMF=K
units=K
nameECMF=Temperature
name=Temperature
cfNameECMF=air_temperature
cfName=air_temperature
cfVarNameECMF=t
cfVarName=t
ifsParam=130
PVPresent=0
deletePV=1
lengthOfHeaders=127
sectionNumber=[1 3 3 4 5 6 7]
section5Length=21
numberOfSection=[1 3 4 5 6 7]
numberOfValues=259920
dataRepresentationTemplateNumber=0
packingType=grid_simple
referenceValue=19690214.0
referenceValueError=2.0
binaryScaleFactor=0
decimalScaleFactor=5
bitsPerValue=24
typeOfOriginalFieldValues=0
sectionNumber=[1 3 3 4 5 6 7]
section6Length=6
numberOfSection=[1 3 4 5 6 7]
bitMapIndicator=255
bitmapPresent=0
sectionNumber=[1 3 3 4 5 6 7]
section7Length=779765
numberOfSection=[1 3 4 5 6 7]
codedValues=[ 257.86242  257.86242  257.86242 ...,  203.76511  203.76511  203.76511]
values=[[ 257.86242  257.86242  257.86242 ...,  257.86242  257.86242  257.86242]
 [ 257.976    257.98259  257.98922 ...,  257.95546  257.9623   257.96917]
 [ 258.08455  258.10058  258.11666 ...,  258.03496  258.0515   258.06807]
 ..., 
 [ 204.1485   204.14371  204.13889 ...,  204.16214  204.1576   204.15304]
 [ 203.97674  203.97458  203.97242 ...,  203.98269  203.9807   203.97872]
 [ 203.76511  203.76511  203.76511 ...,  203.76511  203.76511  203.76511]]
packingError=1.5000000000000002e-05
unpackedError=2.0
maximum=259.82811000000004
minimum=196.90214
average=227.52523846314816
numberOfMissing=0
standardDeviation=13.004208405462618
skewness=0.006497730485033669
kurtosis=0.19096496060806878
isConstant=0.0
changeDecimalPrecision=5
decimalPrecision=5
setBitsPerValue=24
getNumberOfValues=259920
scaleValuesBy=1.0
offsetValuesBy=0.0
productType=unknown
section8Length=4
parametersVersion=1
UseEcmfConventions=1
GRIBEX_boustrophedonic=0
hundred=[100 100]
globalDomain=g
GRIBEditionNumber=2
grib2divider=1000000
angularPrecision=1000000
missingValue=9999
ieeeFloats=1
isHindcast=0
section0Length=16
identifier=GRIB
discipline=0
editionNumber=2
totalLength=779939
sectionNumber=[1 3 3 4 5 6 7]
section1Length=21
numberOfSection=[1 3 4 5 6 7]
centre=0
centreDescription=Absent
subCentre=0
tablesVersion=1
masterDir=grib2/tables/[tablesVersion]
localTablesVersion=0
significanceOfReferenceTime=1
year=2016
month=11
day=1
hour=12
minute=0
second=0
dataDate=20161101
julianDay=2457694.0
dataTime=1200
productionStatusOfProcessedData=0
typeOfProcessedData=af
selectStepTemplateInterval=1
selectStepTemplateInstant=1
stepType=instant
setCalendarId=0
deleteCalendarId=0
sectionNumber=[1 3 3 4 5 6 7]
grib2LocalSectionPresent=0
sectionNumber=[1 3 3 4 5 6 7]
gridDescriptionSectionPresent=1
section3Length=72
numberOfSection=[1 3 4 5 6 7]
sourceOfGridDefinition=0
numberOfDataPoints=259920
numberOfOctectsForNumberOfPoints=0
interpretationOfNumberOfPoints=0
PLPresent=0
gridDefinitionTemplateNumber=0
gridDefinitionDescription=Latitude/longitude. Also called equidistant cylindrical, or Plate Carree
shapeOfTheEarth=6
scaleFactorOfRadiusOfSphericalEarth=0
scaledValueOfRadiusOfSphericalEarth=0
scaleFactorOfEarthMajorAxis=0
scaledValueOfEarthMajorAxis=0
scaleFactorOfEarthMinorAxis=0
scaledValueOfEarthMinorAxis=0
radius=6371229
Ni=720
Nj=361
basicAngleOfTheInitialProductionDomain=0
mBasicAngle=0
angleMultiplier=1
mAngleMultiplier=1000000
subdivisionsOfBasicAngle=0
angleDivisor=1000000
latitudeOfFirstGridPoint=-90000000
longitudeOfFirstGridPoint=0
resolutionAndComponentFlags=48
resolutionAndComponentFlags1=0
resolutionAndComponentFlags2=0
iDirectionIncrementGiven=1
jDirectionIncrementGiven=1
uvRelativeToGrid=0
resolutionAndComponentFlags6=0
resolutionAndComponentFlags7=0
resolutionAndComponentFlags8=0
ijDirectionIncrementGiven=1
latitudeOfLastGridPoint=90000000
longitudeOfLastGridPoint=359500000
iDirectionIncrement=500000
jDirectionIncrement=500000
scanningMode=64
iScansNegatively=0
jScansPositively=1
jPointsAreConsecutive=0
alternativeRowScanning=0
iScansPositively=1
scanningMode5=0
scanningMode6=0
scanningMode7=0
scanningMode8=0
g2grid=[ -90.     0.    90.   359.5    0.5    0.5]
latitudeOfFirstGridPointInDegrees=-90.0
longitudeOfFirstGridPointInDegrees=0.0
latitudeOfLastGridPointInDegrees=90.0
longitudeOfLastGridPointInDegrees=359.5
iDirectionIncrementInDegrees=0.5
jDirectionIncrementInDegrees=0.5
latLonValues=[ -9.00000000e+01   0.00000000e+00  -1.00000000e-05 ...,   9.00000000e+01
   3.59500000e+02  -1.00000000e-05]
latitudes=[-90. -90. -90. ...,  90.  90.  90.]
longitudes=[   0.     0.5    1.  ...,  358.5  359.   359.5]
distinctLatitudes=[-90.  -89.5 -89.  -88.5 -88.  -87.5 -87.  -86.5 -86.  -85.5 -85.  -84.5
 -84.  -83.5 -83.  -82.5 -82.  -81.5 -81.  -80.5 -80.  -79.5 -79.  -78.5
 -78.  -77.5 -77.  -76.5 -76.  -75.5 -75.  -74.5 -74.  -73.5 -73.  -72.5
 -72.  -71.5 -71.  -70.5 -70.  -69.5 -69.  -68.5 -68.  -67.5 -67.  -66.5
 -66.  -65.5 -65.  -64.5 -64.  -63.5 -63.  -62.5 -62.  -61.5 -61.  -60.5
 -60.  -59.5 -59.  -58.5 -58.  -57.5 -57.  -56.5 -56.  -55.5 -55.  -54.5
 -54.  -53.5 -53.  -52.5 -52.  -51.5 -51.  -50.5 -50.  -49.5 -49.  -48.5
 -48.  -47.5 -47.  -46.5 -46.  -45.5 -45.  -44.5 -44.  -43.5 -43.  -42.5
 -42.  -41.5 -41.  -40.5 -40.  -39.5 -39.  -38.5 -38.  -37.5 -37.  -36.5
 -36.  -35.5 -35.  -34.5 -34.  -33.5 -33.  -32.5 -32.  -31.5 -31.  -30.5
 -30.  -29.5 -29.  -28.5 -28.  -27.5 -27.  -26.5 -26.  -25.5 -25.  -24.5
 -24.  -23.5 -23.  -22.5 -22.  -21.5 -21.  -20.5 -20.  -19.5 -19.  -18.5
 -18.  -17.5 -17.  -16.5 -16.  -15.5 -15.  -14.5 -14.  -13.5 -13.  -12.5
 -12.  -11.5 -11.  -10.5 -10.   -9.5  -9.   -8.5  -8.   -7.5  -7.   -6.5
  -6.   -5.5  -5.   -4.5  -4.   -3.5  -3.   -2.5  -2.   -1.5  -1.   -0.5
   0.    0.5   1.    1.5   2.    2.5   3.    3.5   4.    4.5   5.    5.5
   6.    6.5   7.    7.5   8.    8.5   9.    9.5  10.   10.5  11.   11.5
  12.   12.5  13.   13.5  14.   14.5  15.   15.5  16.   16.5  17.   17.5
  18.   18.5  19.   19.5  20.   20.5  21.   21.5  22.   22.5  23.   23.5
  24.   24.5  25.   25.5  26.   26.5  27.   27.5  28.   28.5  29.   29.5
  30.   30.5  31.   31.5  32.   32.5  33.   33.5  34.   34.5  35.   35.5
  36.   36.5  37.   37.5  38.   38.5  39.   39.5  40.   40.5  41.   41.5
  42.   42.5  43.   43.5  44.   44.5  45.   45.5  46.   46.5  47.   47.5
  48.   48.5  49.   49.5  50.   50.5  51.   51.5  52.   52.5  53.   53.5
  54.   54.5  55.   55.5  56.   56.5  57.   57.5  58.   58.5  59.   59.5
  60.   60.5  61.   61.5  62.   62.5  63.   63.5  64.   64.5  65.   65.5
  66.   66.5  67.   67.5  68.   68.5  69.   69.5  70.   70.5  71.   71.5
  72.   72.5  73.   73.5  74.   74.5  75.   75.5  76.   76.5  77.   77.5
  78.   78.5  79.   79.5  80.   80.5  81.   81.5  82.   82.5  83.   83.5
  84.   84.5  85.   85.5  86.   86.5  87.   87.5  88.   88.5  89.   89.5
  90. ]
distinctLongitudes=[   0.     0.5    1.     1.5    2.     2.5    3.     3.5    4.     4.5
    5.     5.5    6.     6.5    7.     7.5    8.     8.5    9.     9.5
   10.    10.5   11.    11.5   12.    12.5   13.    13.5   14.    14.5
   15.    15.5   16.    16.5   17.    17.5   18.    18.5   19.    19.5
   20.    20.5   21.    21.5   22.    22.5   23.    23.5   24.    24.5
   25.    25.5   26.    26.5   27.    27.5   28.    28.5   29.    29.5
   30.    30.5   31.    31.5   32.    32.5   33.    33.5   34.    34.5
   35.    35.5   36.    36.5   37.    37.5   38.    38.5   39.    39.5
   40.    40.5   41.    41.5   42.    42.5   43.    43.5   44.    44.5
   45.    45.5   46.    46.5   47.    47.5   48.    48.5   49.    49.5
   50.    50.5   51.    51.5   52.    52.5   53.    53.5   54.    54.5
   55.    55.5   56.    56.5   57.    57.5   58.    58.5   59.    59.5
   60.    60.5   61.    61.5   62.    62.5   63.    63.5   64.    64.5
   65.    65.5   66.    66.5   67.    67.5   68.    68.5   69.    69.5
   70.    70.5   71.    71.5   72.    72.5   73.    73.5   74.    74.5
   75.    75.5   76.    76.5   77.    77.5   78.    78.5   79.    79.5
   80.    80.5   81.    81.5   82.    82.5   83.    83.5   84.    84.5
   85.    85.5   86.    86.5   87.    87.5   88.    88.5   89.    89.5
   90.    90.5   91.    91.5   92.    92.5   93.    93.5   94.    94.5
   95.    95.5   96.    96.5   97.    97.5   98.    98.5   99.    99.5
  100.   100.5  101.   101.5  102.   102.5  103.   103.5  104.   104.5
  105.   105.5  106.   106.5  107.   107.5  108.   108.5  109.   109.5
  110.   110.5  111.   111.5  112.   112.5  113.   113.5  114.   114.5
  115.   115.5  116.   116.5  117.   117.5  118.   118.5  119.   119.5
  120.   120.5  121.   121.5  122.   122.5  123.   123.5  124.   124.5
  125.   125.5  126.   126.5  127.   127.5  128.   128.5  129.   129.5
  130.   130.5  131.   131.5  132.   132.5  133.   133.5  134.   134.5
  135.   135.5  136.   136.5  137.   137.5  138.   138.5  139.   139.5
  140.   140.5  141.   141.5  142.   142.5  143.   143.5  144.   144.5
  145.   145.5  146.   146.5  147.   147.5  148.   148.5  149.   149.5
  150.   150.5  151.   151.5  152.   152.5  153.   153.5  154.   154.5
  155.   155.5  156.   156.5  157.   157.5  158.   158.5  159.   159.5
  160.   160.5  161.   161.5  162.   162.5  163.   163.5  164.   164.5
  165.   165.5  166.   166.5  167.   167.5  168.   168.5  169.   169.5
  170.   170.5  171.   171.5  172.   172.5  173.   173.5  174.   174.5
  175.   175.5  176.   176.5  177.   177.5  178.   178.5  179.   179.5
  180.   180.5  181.   181.5  182.   182.5  183.   183.5  184.   184.5
  185.   185.5  186.   186.5  187.   187.5  188.   188.5  189.   189.5
  190.   190.5  191.   191.5  192.   192.5  193.   193.5  194.   194.5
  195.   195.5  196.   196.5  197.   197.5  198.   198.5  199.   199.5
  200.   200.5  201.   201.5  202.   202.5  203.   203.5  204.   204.5
  205.   205.5  206.   206.5  207.   207.5  208.   208.5  209.   209.5
  210.   210.5  211.   211.5  212.   212.5  213.   213.5  214.   214.5
  215.   215.5  216.   216.5  217.   217.5  218.   218.5  219.   219.5
  220.   220.5  221.   221.5  222.   222.5  223.   223.5  224.   224.5
  225.   225.5  226.   226.5  227.   227.5  228.   228.5  229.   229.5
  230.   230.5  231.   231.5  232.   232.5  233.   233.5  234.   234.5
  235.   235.5  236.   236.5  237.   237.5  238.   238.5  239.   239.5
  240.   240.5  241.   241.5  242.   242.5  243.   243.5  244.   244.5
  245.   245.5  246.   246.5  247.   247.5  248.   248.5  249.   249.5
  250.   250.5  251.   251.5  252.   252.5  253.   253.5  254.   254.5
  255.   255.5  256.   256.5  257.   257.5  258.   258.5  259.   259.5
  260.   260.5  261.   261.5  262.   262.5  263.   263.5  264.   264.5
  265.   265.5  266.   266.5  267.   267.5  268.   268.5  269.   269.5
  270.   270.5  271.   271.5  272.   272.5  273.   273.5  274.   274.5
  275.   275.5  276.   276.5  277.   277.5  278.   278.5  279.   279.5
  280.   280.5  281.   281.5  282.   282.5  283.   283.5  284.   284.5
  285.   285.5  286.   286.5  287.   287.5  288.   288.5  289.   289.5
  290.   290.5  291.   291.5  292.   292.5  293.   293.5  294.   294.5
  295.   295.5  296.   296.5  297.   297.5  298.   298.5  299.   299.5
  300.   300.5  301.   301.5  302.   302.5  303.   303.5  304.   304.5
  305.   305.5  306.   306.5  307.   307.5  308.   308.5  309.   309.5
  310.   310.5  311.   311.5  312.   312.5  313.   313.5  314.   314.5
  315.   315.5  316.   316.5  317.   317.5  318.   318.5  319.   319.5
  320.   320.5  321.   321.5  322.   322.5  323.   323.5  324.   324.5
  325.   325.5  326.   326.5  327.   327.5  328.   328.5  329.   329.5
  330.   330.5  331.   331.5  332.   332.5  333.   333.5  334.   334.5
  335.   335.5  336.   336.5  337.   337.5  338.   338.5  339.   339.5
  340.   340.5  341.   341.5  342.   342.5  343.   343.5  344.   344.5
  345.   345.5  346.   346.5  347.   347.5  348.   348.5  349.   349.5
  350.   350.5  351.   351.5  352.   352.5  353.   353.5  354.   354.5
  355.   355.5  356.   356.5  357.   357.5  358.   358.5  359.   359.5]
gridType=regular_ll
sectionNumber=[1 3 3 4 5 6 7]
section4Length=34
numberOfSection=[1 3 4 5 6 7]
NV=0
neitherPresent=0
productDefinitionTemplateNumber=0
genVertHeightCoords=0
parameterCategory=2
parameterNumber=2
parameterUnits=m s-1
parameterName=u-component of wind
typeOfGeneratingProcess=2
backgroundProcess=0
generatingProcessIdentifier=0
hoursAfterDataCutoff=4294967295
minutesAfterDataCutoff=4294967295
indicatorOfUnitOfTimeRange=1
stepUnits=1
forecastTime=0
startStep=0
endStep=0
stepRange=0
stepTypeInternal=instant
validityDate=20161101
validityTime=1200
typeOfFirstFixedSurface=pl
unitsOfFirstFixedSurface=Pa
nameOfFirstFixedSurface=Isobaric surface
scaleFactorOfFirstFixedSurface=-2
scaledValueOfFirstFixedSurface=10
typeOfSecondFixedSurface=255
unitsOfSecondFixedSurface=unknown
nameOfSecondFixedSurface=Missing
scaleFactorOfSecondFixedSurface=0
scaledValueOfSecondFixedSurface=0
pressureUnits=hPa
typeOfLevel=isobaricInhPa
level=10
bottomLevel=10
topLevel=10
tempPressureUnits=hPa
paramIdECMF=131
paramId=131
shortNameECMF=u
shortName=u
unitsECMF=m s**-1
units=m s**-1
nameECMF=U component of wind
name=U component of wind
cfNameECMF=eastward_wind
cfName=eastward_wind
cfVarNameECMF=u
cfVarName=u
ifsParam=131
PVPresent=0
deletePV=1
lengthOfHeaders=127
sectionNumber=[1 3 3 4 5 6 7]
section5Length=21
numberOfSection=[1 3 4 5 6 7]
numberOfValues=259920
dataRepresentationTemplateNumber=0
packingType=grid_simple
referenceValue=-3171082.0
referenceValueError=0.25
binaryScaleFactor=0
decimalScaleFactor=5
bitsPerValue=24
typeOfOriginalFieldValues=0
sectionNumber=[1 3 3 4 5 6 7]
section6Length=6
numberOfSection=[1 3 4 5 6 7]
bitMapIndicator=255
bitmapPresent=0
sectionNumber=[1 3 3 4 5 6 7]
section7Length=779765
numberOfSection=[1 3 4 5 6 7]
codedValues=[ -1.00000000e-05  -1.00000000e-05  -1.00000000e-05 ...,  -1.00000000e-05
  -1.00000000e-05  -1.00000000e-05]
values=[[ -1.00000000e-05  -1.00000000e-05  -1.00000000e-05 ...,  -1.00000000e-05
   -1.00000000e-05  -1.00000000e-05]
 [ -2.24515100e+01  -2.21896500e+01  -2.19775600e+01 ...,  -1.61150000e+00
   -1.58409000e+00  -1.85152000e+00]
 [ -2.20500300e+01  -2.17862600e+01  -2.15759800e+01 ...,   2.55070000e-01
    3.11640000e-01   5.15300000e-02]
 ..., 
 [ -1.31155400e+01  -1.30517100e+01  -1.29987900e+01 ...,  -8.70033000e+00
   -8.63194000e+00  -8.62856000e+00]
 [ -1.35571200e+01  -1.34884100e+01  -1.34347000e+01 ...,  -7.41520000e+00
   -7.35897000e+00  -7.39162000e+00]
 [ -1.00000000e-05  -1.00000000e-05  -1.00000000e-05 ...,  -1.00000000e-05
   -1.00000000e-05  -1.00000000e-05]]
packingError=6.25e-06
unpackedError=0.25
maximum=50.503930000000004
minimum=-31.710820000000002
average=1.4625147551938655
numberOfMissing=0
standardDeviation=12.399939498032508
skewness=0.7829871772412588
kurtosis=1.3186787298825253
isConstant=0.0
changeDecimalPrecision=5
decimalPrecision=5
setBitsPerValue=24
getNumberOfValues=259920
scaleValuesBy=1.0
offsetValuesBy=0.0
productType=unknown
section8Length=4
parametersVersion=1
UseEcmfConventions=1
GRIBEX_boustrophedonic=0
hundred=[100 100]
globalDomain=g
GRIBEditionNumber=2
grib2divider=1000000
angularPrecision=1000000
missingValue=9999
ieeeFloats=1
isHindcast=0
section0Length=16
identifier=GRIB
discipline=0
editionNumber=2
totalLength=779939
sectionNumber=[1 3 3 4 5 6 7]
section1Length=21
numberOfSection=[1 3 4 5 6 7]
centre=0
centreDescription=Absent
subCentre=0
tablesVersion=1
masterDir=grib2/tables/[tablesVersion]
localTablesVersion=0
significanceOfReferenceTime=1
year=2016
month=11
day=1
hour=12
minute=0
second=0
dataDate=20161101
julianDay=2457694.0
dataTime=1200
productionStatusOfProcessedData=0
typeOfProcessedData=af
selectStepTemplateInterval=1
selectStepTemplateInstant=1
stepType=instant
setCalendarId=0
deleteCalendarId=0
sectionNumber=[1 3 3 4 5 6 7]
grib2LocalSectionPresent=0
sectionNumber=[1 3 3 4 5 6 7]
gridDescriptionSectionPresent=1
section3Length=72
numberOfSection=[1 3 4 5 6 7]
sourceOfGridDefinition=0
numberOfDataPoints=259920
numberOfOctectsForNumberOfPoints=0
interpretationOfNumberOfPoints=0
PLPresent=0
gridDefinitionTemplateNumber=0
gridDefinitionDescription=Latitude/longitude. Also called equidistant cylindrical, or Plate Carree
shapeOfTheEarth=6
scaleFactorOfRadiusOfSphericalEarth=0
scaledValueOfRadiusOfSphericalEarth=0
scaleFactorOfEarthMajorAxis=0
scaledValueOfEarthMajorAxis=0
scaleFactorOfEarthMinorAxis=0
scaledValueOfEarthMinorAxis=0
radius=6371229
Ni=720
Nj=361
basicAngleOfTheInitialProductionDomain=0
mBasicAngle=0
angleMultiplier=1
mAngleMultiplier=1000000
subdivisionsOfBasicAngle=0
angleDivisor=1000000
latitudeOfFirstGridPoint=-90000000
longitudeOfFirstGridPoint=0
resolutionAndComponentFlags=48
resolutionAndComponentFlags1=0
resolutionAndComponentFlags2=0
iDirectionIncrementGiven=1
jDirectionIncrementGiven=1
uvRelativeToGrid=0
resolutionAndComponentFlags6=0
resolutionAndComponentFlags7=0
resolutionAndComponentFlags8=0
ijDirectionIncrementGiven=1
latitudeOfLastGridPoint=90000000
longitudeOfLastGridPoint=359500000
iDirectionIncrement=500000
jDirectionIncrement=500000
scanningMode=64
iScansNegatively=0
jScansPositively=1
jPointsAreConsecutive=0
alternativeRowScanning=0
iScansPositively=1
scanningMode5=0
scanningMode6=0
scanningMode7=0
scanningMode8=0
g2grid=[ -90.     0.    90.   359.5    0.5    0.5]
latitudeOfFirstGridPointInDegrees=-90.0
longitudeOfFirstGridPointInDegrees=0.0
latitudeOfLastGridPointInDegrees=90.0
longitudeOfLastGridPointInDegrees=359.5
iDirectionIncrementInDegrees=0.5
jDirectionIncrementInDegrees=0.5
latLonValues=[ -9.00000000e+01   0.00000000e+00  -1.00000000e-05 ...,   9.00000000e+01
   3.59500000e+02  -1.00000000e-05]
latitudes=[-90. -90. -90. ...,  90.  90.  90.]
longitudes=[   0.     0.5    1.  ...,  358.5  359.   359.5]
distinctLatitudes=
[-90.  -89.5 -89.  -88.5 -88.  -87.5 -87.  -86.5 -86.  -85.5 -85.  -84.5
 -84.  -83.5 -83.  -82.5 -82.  -81.5 -81.  -80.5 -80.  -79.5 -79.  -78.5
 -78.  -77.5 -77.  -76.5 -76.  -75.5 -75.  -74.5 -74.  -73.5 -73.  -72.5
 -72.  -71.5 -71.  -70.5 -70.  -69.5 -69.  -68.5 -68.  -67.5 -67.  -66.5
 -66.  -65.5 -65.  -64.5 -64.  -63.5 -63.  -62.5 -62.  -61.5 -61.  -60.5
 -60.  -59.5 -59.  -58.5 -58.  -57.5 -57.  -56.5 -56.  -55.5 -55.  -54.5
 -54.  -53.5 -53.  -52.5 -52.  -51.5 -51.  -50.5 -50.  -49.5 -49.  -48.5
 -48.  -47.5 -47.  -46.5 -46.  -45.5 -45.  -44.5 -44.  -43.5 -43.  -42.5
 -42.  -41.5 -41.  -40.5 -40.  -39.5 -39.  -38.5 -38.  -37.5 -37.  -36.5
 -36.  -35.5 -35.  -34.5 -34.  -33.5 -33.  -32.5 -32.  -31.5 -31.  -30.5
 -30.  -29.5 -29.  -28.5 -28.  -27.5 -27.  -26.5 -26.  -25.5 -25.  -24.5
 -24.  -23.5 -23.  -22.5 -22.  -21.5 -21.  -20.5 -20.  -19.5 -19.  -18.5
 -18.  -17.5 -17.  -16.5 -16.  -15.5 -15.  -14.5 -14.  -13.5 -13.  -12.5
 -12.  -11.5 -11.  -10.5 -10.   -9.5  -9.   -8.5  -8.   -7.5  -7.   -6.5
  -6.   -5.5  -5.   -4.5  -4.   -3.5  -3.   -2.5  -2.   -1.5  -1.   -0.5
   0.    0.5   1.    1.5   2.    2.5   3.    3.5   4.    4.5   5.    5.5
   6.    6.5   7.    7.5   8.    8.5   9.    9.5  10.   10.5  11.   11.5
  12.   12.5  13.   13.5  14.   14.5  15.   15.5  16.   16.5  17.   17.5
  18.   18.5  19.   19.5  20.   20.5  21.   21.5  22.   22.5  23.   23.5
  24.   24.5  25.   25.5  26.   26.5  27.   27.5  28.   28.5  29.   29.5
  30.   30.5  31.   31.5  32.   32.5  33.   33.5  34.   34.5  35.   35.5
  36.   36.5  37.   37.5  38.   38.5  39.   39.5  40.   40.5  41.   41.5
  42.   42.5  43.   43.5  44.   44.5  45.   45.5  46.   46.5  47.   47.5
  48.   48.5  49.   49.5  50.   50.5  51.   51.5  52.   52.5  53.   53.5
  54.   54.5  55.   55.5  56.   56.5  57.   57.5  58.   58.5  59.   59.5
  60.   60.5  61.   61.5  62.   62.5  63.   63.5  64.   64.5  65.   65.5
  66.   66.5  67.   67.5  68.   68.5  69.   69.5  70.   70.5  71.   71.5
  72.   72.5  73.   73.5  74.   74.5  75.   75.5  76.   76.5  77.   77.5
  78.   78.5  79.   79.5  80.   80.5  81.   81.5  82.   82.5  83.   83.5
  84.   84.5  85.   85.5  86.   86.5  87.   87.5  88.   88.5  89.   89.5
  90. ]
distinctLongitudes=[   0.     0.5    1.     1.5    2.     2.5    3.     3.5    4.     4.5
    5.     5.5    6.     6.5    7.     7.5    8.     8.5    9.     9.5
   10.    10.5   11.    11.5   12.    12.5   13.    13.5   14.    14.5
   15.    15.5   16.    16.5   17.    17.5   18.    18.5   19.    19.5
   20.    20.5   21.    21.5   22.    22.5   23.    23.5   24.    24.5
   25.    25.5   26.    26.5   27.    27.5   28.    28.5   29.    29.5
   30.    30.5   31.    31.5   32.    32.5   33.    33.5   34.    34.5
   35.    35.5   36.    36.5   37.    37.5   38.    38.5   39.    39.5
   40.    40.5   41.    41.5   42.    42.5   43.    43.5   44.    44.5
   45.    45.5   46.    46.5   47.    47.5   48.    48.5   49.    49.5
   50.    50.5   51.    51.5   52.    52.5   53.    53.5   54.    54.5
   55.    55.5   56.    56.5   57.    57.5   58.    58.5   59.    59.5
   60.    60.5   61.    61.5   62.    62.5   63.    63.5   64.    64.5
   65.    65.5   66.    66.5   67.    67.5   68.    68.5   69.    69.5
   70.    70.5   71.    71.5   72.    72.5   73.    73.5   74.    74.5
   75.    75.5   76.    76.5   77.    77.5   78.    78.5   79.    79.5
   80.    80.5   81.    81.5   82.    82.5   83.    83.5   84.    84.5
   85.    85.5   86.    86.5   87.    87.5   88.    88.5   89.    89.5
   90.    90.5   91.    91.5   92.    92.5   93.    93.5   94.    94.5
   95.    95.5   96.    96.5   97.    97.5   98.    98.5   99.    99.5
  100.   100.5  101.   101.5  102.   102.5  103.   103.5  104.   104.5
  105.   105.5  106.   106.5  107.   107.5  108.   108.5  109.   109.5
  110.   110.5  111.   111.5  112.   112.5  113.   113.5  114.   114.5
  115.   115.5  116.   116.5  117.   117.5  118.   118.5  119.   119.5
  120.   120.5  121.   121.5  122.   122.5  123.   123.5  124.   124.5
  125.   125.5  126.   126.5  127.   127.5  128.   128.5  129.   129.5
  130.   130.5  131.   131.5  132.   132.5  133.   133.5  134.   134.5
  135.   135.5  136.   136.5  137.   137.5  138.   138.5  139.   139.5
  140.   140.5  141.   141.5  142.   142.5  143.   143.5  144.   144.5
  145.   145.5  146.   146.5  147.   147.5  148.   148.5  149.   149.5
  150.   150.5  151.   151.5  152.   152.5  153.   153.5  154.   154.5
  155.   155.5  156.   156.5  157.   157.5  158.   158.5  159.   159.5
  160.   160.5  161.   161.5  162.   162.5  163.   163.5  164.   164.5
  165.   165.5  166.   166.5  167.   167.5  168.   168.5  169.   169.5
  170.   170.5  171.   171.5  172.   172.5  173.   173.5  174.   174.5
  175.   175.5  176.   176.5  177.   177.5  178.   178.5  179.   179.5
  180.   180.5  181.   181.5  182.   182.5  183.   183.5  184.   184.5
  185.   185.5  186.   186.5  187.   187.5  188.   188.5  189.   189.5
  190.   190.5  191.   191.5  192.   192.5  193.   193.5  194.   194.5
  195.   195.5  196.   196.5  197.   197.5  198.   198.5  199.   199.5
  200.   200.5  201.   201.5  202.   202.5  203.   203.5  204.   204.5
  205.   205.5  206.   206.5  207.   207.5  208.   208.5  209.   209.5
  210.   210.5  211.   211.5  212.   212.5  213.   213.5  214.   214.5
  215.   215.5  216.   216.5  217.   217.5  218.   218.5  219.   219.5
  220.   220.5  221.   221.5  222.   222.5  223.   223.5  224.   224.5
  225.   225.5  226.   226.5  227.   227.5  228.   228.5  229.   229.5
  230.   230.5  231.   231.5  232.   232.5  233.   233.5  234.   234.5
  235.   235.5  236.   236.5  237.   237.5  238.   238.5  239.   239.5
  240.   240.5  241.   241.5  242.   242.5  243.   243.5  244.   244.5
  245.   245.5  246.   246.5  247.   247.5  248.   248.5  249.   249.5
  250.   250.5  251.   251.5  252.   252.5  253.   253.5  254.   254.5
  255.   255.5  256.   256.5  257.   257.5  258.   258.5  259.   259.5
  260.   260.5  261.   261.5  262.   262.5  263.   263.5  264.   264.5
  265.   265.5  266.   266.5  267.   267.5  268.   268.5  269.   269.5
  270.   270.5  271.   271.5  272.   272.5  273.   273.5  274.   274.5
  275.   275.5  276.   276.5  277.   277.5  278.   278.5  279.   279.5
  280.   280.5  281.   281.5  282.   282.5  283.   283.5  284.   284.5
  285.   285.5  286.   286.5  287.   287.5  288.   288.5  289.   289.5
  290.   290.5  291.   291.5  292.   292.5  293.   293.5  294.   294.5
  295.   295.5  296.   296.5  297.   297.5  298.   298.5  299.   299.5
  300.   300.5  301.   301.5  302.   302.5  303.   303.5  304.   304.5
  305.   305.5  306.   306.5  307.   307.5  308.   308.5  309.   309.5
  310.   310.5  311.   311.5  312.   312.5  313.   313.5  314.   314.5
  315.   315.5  316.   316.5  317.   317.5  318.   318.5  319.   319.5
  320.   320.5  321.   321.5  322.   322.5  323.   323.5  324.   324.5
  325.   325.5  326.   326.5  327.   327.5  328.   328.5  329.   329.5
  330.   330.5  331.   331.5  332.   332.5  333.   333.5  334.   334.5
  335.   335.5  336.   336.5  337.   337.5  338.   338.5  339.   339.5
  340.   340.5  341.   341.5  342.   342.5  343.   343.5  344.   344.5
  345.   345.5  346.   346.5  347.   347.5  348.   348.5  349.   349.5
  350.   350.5  351.   351.5  352.   352.5  353.   353.5  354.   354.5
  355.   355.5  356.   356.5  357.   357.5  358.   358.5  359.   359.5]
gridType=regular_ll
sectionNumber=[1 3 3 4 5 6 7]
section4Length=34
numberOfSection=[1 3 4 5 6 7]
NV=0
neitherPresent=0
productDefinitionTemplateNumber=0
genVertHeightCoords=0
parameterCategory=2
parameterNumber=3
parameterUnits=m s-1
parameterName=v-component of wind
typeOfGeneratingProcess=2
backgroundProcess=0
generatingProcessIdentifier=0
hoursAfterDataCutoff=4294967295
minutesAfterDataCutoff=4294967295
indicatorOfUnitOfTimeRange=1
stepUnits=1
forecastTime=0
startStep=0
endStep=0
stepRange=0
stepTypeInternal=instant
validityDate=20161101
validityTime=1200
typeOfFirstFixedSurface=pl
unitsOfFirstFixedSurface=Pa
nameOfFirstFixedSurface=Isobaric surface
scaleFactorOfFirstFixedSurface=-2
scaledValueOfFirstFixedSurface=10
typeOfSecondFixedSurface=255
unitsOfSecondFixedSurface=unknown
nameOfSecondFixedSurface=Missing
scaleFactorOfSecondFixedSurface=0
scaledValueOfSecondFixedSurface=0
pressureUnits=hPa
typeOfLevel=isobaricInhPa
level=10
bottomLevel=10
topLevel=10
tempPressureUnits=hPa
paramIdECMF=132
paramId=132
shortNameECMF=v
shortName=v
unitsECMF=m s**-1
units=m s**-1
nameECMF=V component of wind
name=V component of wind
cfNameECMF=northward_wind
cfName=northward_wind
cfVarNameECMF=v
cfVarName=v
ifsParam=132
PVPresent=0
deletePV=1
lengthOfHeaders=127
sectionNumber=[1 3 3 4 5 6 7]
section5Length=21
numberOfSection=[1 3 4 5 6 7]
numberOfValues=259920
dataRepresentationTemplateNumber=0
packingType=grid_simple
referenceValue=-4472193.0
referenceValueError=0.5
binaryScaleFactor=0
decimalScaleFactor=5
bitsPerValue=24
typeOfOriginalFieldValues=0
sectionNumber=[1 3 3 4 5 6 7]
section6Length=6
numberOfSection=[1 3 4 5 6 7]
bitMapIndicator=255
bitmapPresent=0
sectionNumber=[1 3 3 4 5 6 7]
section7Length=779765
numberOfSection=[1 3 4 5 6 7]
codedValues=[ -1.00000000e-05  -1.00000000e-05  -1.00000000e-05 ...,  -1.00000000e-05
  -1.00000000e-05  -1.00000000e-05]
values=[[ -1.00000000e-05  -1.00000000e-05  -1.00000000e-05 ...,  -1.00000000e-05
   -1.00000000e-05  -1.00000000e-05]
 [ -2.60293000e+01  -2.62055400e+01  -2.64316900e+01 ...,  -5.50365000e+00
   -5.56461000e+00  -5.91251000e+00]
 [ -2.66242400e+01  -2.68222700e+01  -2.70680500e+01 ...,  -7.02217000e+00
   -7.08184000e+00  -7.41549000e+00]
 ..., 
 [  5.58142000e+00   5.69179000e+00   5.79713000e+00 ...,   7.64182000e+00
    7.70772000e+00   7.74192000e+00]
 [  6.00146000e+00   6.11658000e+00   6.22818000e+00 ...,   7.44852000e+00
    7.51078000e+00   7.55014000e+00]
 [ -1.00000000e-05  -1.00000000e-05  -1.00000000e-05 ...,  -1.00000000e-05
   -1.00000000e-05  -1.00000000e-05]]
packingError=7.500000000000001e-06
unpackedError=0.5
maximum=45.09022
minimum=-44.72193
average=-1.2278746843260007
numberOfMissing=0
standardDeviation=9.946358900803972
skewness=-0.5671987656493384
kurtosis=3.938821000700548
isConstant=0.0
changeDecimalPrecision=5
decimalPrecision=5
setBitsPerValue=24
getNumberOfValues=259920
scaleValuesBy=1.0
offsetValuesBy=0.0
productType=unknown
section8Length=4
"""