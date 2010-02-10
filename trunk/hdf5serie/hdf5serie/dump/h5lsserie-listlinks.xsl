<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:hdf5="http://hdfgroup.org/DTDs/HDF5-File">

  <xsl:output method="text"/>

  <xsl:template match="text()"/>

  <!-- recursion -->
  <xsl:template match="*">
    <xsl:apply-templates/>
  </xsl:template>

  <!-- output H5:ExternalLink -->
  <xsl:template match="hdf5:ExternalLink">
    <xsl:value-of select="translate(concat(@TargetFilename,@TargetPath),'/','')"/><xsl:text> </xsl:text>
  </xsl:template>

</xsl:stylesheet>
