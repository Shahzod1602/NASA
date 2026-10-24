PROJECT NAME
  Apollo 12 Landing Site

LROC NAC SOURCE IMAGES
  M120005333 L/R
  M120012135 L/R

TECHNICAL CONTACT INFORMATION
  NAME		ARIZONA STATE UNIVERSITY
  INSTITUTION	ARIZONA STATE UNIVERSITY

DATE CREATED
  September  5, 2013

- Products are tied to LOLA elevations with RMS error of 1.31 m to 19.0 different LOLA orbit tracks.
  The RMS error is the root-mean-square of the elevation difference between the LOLA points and the DTM. This value
  is a combination of error from the LOLA points and the DTM, and may give a measure of the horizontal and vertical
  accuracy of the DTM.
- The reported precision error from SOCET SET is 0.8 m.
- SOCET SET NGATE v5.5 was used to extract terrain.
- Post spacing in DTM is 2.0 m.
- DTM start time: 2010-02-05 10:34:26 and DTM stop time: 2010-02-05 12:28:05.
- DTM extents:
   Min. Lat.: -3.05226826
   Min. Lon.: 336.45205417
   Max. Lat.: -2.96217267
   Max. Lon.: 336.61505963

KEY FEATURES
- Mare
- Anthropogenic

REFERENCE
For more information on the methodology and error analysis of the DTMs, see:

M.R. Henriksen, M.R. Manheim, K.N. Burns, P. Seymour, E.J. Speyerer, A. Deran, A.K. Boyd, E. Howington-Kraus, M.R. Rosiek, B.A. Archinal, and M.S. Robinson. Extracting accurate and precise topography from LROC narrow angle camera stereo observations. Icarus (2016). http://dx.doi.org/10.1016/j.icarus.2016.05.012
RDR SIS: http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/DOCUMENT/RDRSIS.PDF

LINKS TO BROWSE IMAGES
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_CLRDISC_LEGEND.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_CLRDISC.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_CLRGRAD_LEGEND.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_CLRGRAD.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_CONF.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_E030S3365.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_M120005333_2M.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_M120005333_50CM.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_M120012135_2M.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_M120012135_50CM.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_SHADE.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_SLOPE_LEGEND.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12_SLOPE.BROWSE.PNG
http://lroc.sese.asu.edu/data/LRO-L-LROC-5-RDR-V1.0/LROLRC_2001/EXTRAS/BROWSE/NAC_DTM/APOLLO12/NAC_DTM_APOLLO12.BROWSE.PNG

GENERAL NOTES:

KNOWN ARTIFACTS
  These are known artifacts present in some, but not all, LROC NAC DTMs.
  These artifacts may not be present in this DTM! Look at the terrain shaded relief and the confidence map
  to detect these before using the DTM!
  - Boxes
        Some DTMs have rectangular areas that are usually about 0.5-1.0 m different in elevation from the
        surrounding areas. These are artifacts of the processing algorithms used in SOCET SET ((c) BAE Systems).
        There may be groups of these boxes. They are almost impossible to edit out, so the user should look
        for such artifacts in a terrain shaded relief map before using the DTM for analysis.
  - NACL and NACR seams
        There are two NAC cameras (NACL & NACR) on board LRO. Stereo images are collected by acquiring images
        on two different orbits so the total parallax angle is greater than 12 degrees. The overlap between
        the four images (two pairs of NACL and NACR) provides three or four stereo models from which to collect
        elevation data. These stereo models are stitched together into one DTM. In some DTMs, the seams between
        the stereo models may be visible as long lines, usually up to 1 m difference. These seams are difficult
        to remove from the whole DTM.
  - Faceted areas
        Areas that were very bland (low contrast) or deeply shadowed with low contrast and low signal may have
        a "faceted" look to them. This is due to the processing algorithm not able to correlate the images and
        must interpolate to fill in the pixel. The terrain data in these areas are not trustworthy. Use the
        confidence map to look for "suspicious" data points.
  - Noise
        DTMs have varying level of noise from low signal images, low incidence angle, and jitter. The amount
        of noise may be detected using the slope map or the confidence map.

PDS PRODUCT NAMING CONVENTION
  The PRODUCT_ID naming convention for LROC NAC DTMs is: NAC_DTM_xxxxxxxxxxx_AyyyBzzzz.IMG where
        - NAC_DTM = indicating it's a NAC DTM product
        - xxxxxxxxxx = words specifying the sitename (up to 11 characters)
        - A = projection type (E for equirectangular, P for polar stereographic)
        - yyy = center latitude	to the nearest 10th of a degree
        - B = N for northern hemisphere, S for southern hemisphere
        - zzzz = center longitude to the nearest tenth of a degree. The longitude
                domain for all NAC DTMs is 360 positive east.
  Example: 	NAC_DTM_LINNECRATER_E280N0120.IMG is a NAC DTM of Linne Crater located at latitude=28.0N,
                 longitude=12.0E in equirectangular projection.

RELATED DTM PRODUCTS
  For each NAC DTM, we also produce several related products. The naming convention for these products is:
  <basename>_<description> where <basename> refers to NAC_DTM_xxxxxxxxxxx and <description> refers to the
  type of product (see details below).
  - Ortho-rectified images
        We provide 4 ortho-rectified images, a mosaic of the NACL_1 and NACR_1 and a mosaic of NACL_2 and NACR_2
        in two resolutions (DTM resolution and image resolution). The <description> part of the filename is the
        name of the NAC image and the resolution.
  - Colorshades and colorshade legends
        The colorshade images are color-coded topography draped over the shaded relief. The colorshade legend
        explains what the color means. There are two versions: a discrete colorshade that bins the data to 
        emphasize topography, and a gradient colorshade that is a continuous color map.  The <description> for
        the discrete colorshade image is CLRDISC and the discrete colorshade legend is CLRDISC_LEGEND. The 
        <description> for the gradient colorshade image is CLRGRAD and the gradient colorshade legend is 
        CLRGRAD_LEGEND.
  - Color slope map and slope map legend
	       The color slope map is the color-coded slope (in discrete increments) draped over the shaded relief. The
        slope map legend explains what the color means. The <description> for the color slope map is SLOPE and
        the slope map legend is SLOPE_LEGEND.
  - Shaded Relief
        The shaded relief image is generated with the sun azimuth of 315 degrees (northwest) and sun altitude of
        45 degrees. The <description> for the shaded relief image is SHADE
  - DTM in Geotiff format
        We also provide the DTM in GeoTIFF format to allow users more flexibility in using the DTM. There are no
        <description> for this file, and the filename is simply <basename>.tif
  - Readme
        The readme file contains description of the DTM, precision and accuracy information, description of known
        artifacts, naming convention, related products, and confidence table lookup. The <description> for the
        readme file is README.

CONFIDENCE MAP TABLE LOOKUP
  Value	 Description
    0	 NoDATA, outside boundary (e.g. out of stereo pair overlap)
    1	 Shadowed
    2	 Saturated
    3	 Suspicious (edge, corner, did not correlate, etc.)
    4	 Interpolated / Extrapolated (e.g. from neighbor pixels)
  10-14	 Value range of successful correlations
    15	 Manually edited


