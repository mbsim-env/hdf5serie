<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:hdf5="http://hdfgroup.org/DTDs/HDF5-File">

  <xsl:param name="DESCRIPTION"/>
  <xsl:param name="LABEL"/>
  <xsl:param name="FILENAME"/>

  <xsl:output method="text"/>

  <xsl:template match="text()"/>

  <!-- set PATH to name of .h5 file (given by FILENAME) -->
  <xsl:template match="/">
    <xsl:apply-templates>
      <xsl:with-param name="PATH" select="$FILENAME"/>
    </xsl:apply-templates>
  </xsl:template>

  <!-- recursion -->
  <xsl:template match="*">
    <xsl:param name="INDENT"/>
    <xsl:param name="PATH"/>
    <xsl:apply-templates>
      <xsl:with-param name="INDENT" select="$INDENT"/>
      <xsl:with-param name="PATH" select="$PATH"/>
    </xsl:apply-templates>
  </xsl:template>

  <!-- output H5:Group -->
  <xsl:template match="hdf5:Group">
    <xsl:param name="INDENT"/>
    <xsl:param name="PATH"/>
    <xsl:value-of select="$INDENT"/>+ <xsl:value-of select="@Name"/>
<xsl:text>
</xsl:text>
    <!-- apply H5:Attributes; increase indent -->
    <xsl:apply-templates select="hdf5:Attribute">
      <xsl:with-param name="INDENT" select="concat($INDENT,'  ')"/>
    </xsl:apply-templates>
    <!-- apply H5:Dataset; increase indent -->
    <xsl:apply-templates select="hdf5:Dataset">
      <xsl:with-param name="INDENT" select="concat($INDENT,'  ')"/>
      <xsl:with-param name="PATH" select="concat($PATH,'/',@Name)"/>
    </xsl:apply-templates>
    <!-- apply H5:Group; increase indent -->
    <xsl:apply-templates select="hdf5:Group">
      <xsl:with-param name="INDENT" select="concat($INDENT,'  ')"/>
      <xsl:with-param name="PATH" select="concat($PATH,'/',@Name)"/>
    </xsl:apply-templates>
  </xsl:template>

  <!-- output H5:Dataset -->
  <xsl:template match="hdf5:Dataset">
    <xsl:param name="INDENT"/>
    <xsl:param name="PATH"/>
    <!-- output name and full path -->
    <xsl:value-of select="$INDENT"/>- <xsl:value-of select="@Name"/> (Path: <xsl:value-of select="concat($PATH,'/',@Name)"/>)<xsl:text>
</xsl:text>
    <!-- apply H5:Attributes; increase indent -->
    <xsl:apply-templates select="hdf5:Attribute">
      <xsl:with-param name="INDENT" select="concat($INDENT,'  ')"/>
    </xsl:apply-templates>
  </xsl:template>

  <!-- output H5:Attribute -->
  <xsl:template match="hdf5:Attribute">
    <xsl:param name="INDENT"/>
    <!-- output Description if requested and exists -->
    <xsl:if test="$DESCRIPTION=1">
    <xsl:if test="@Name='Description'">
      <xsl:value-of select="$INDENT"/>Description: <xsl:value-of select="normalize-space(hdf5:Data/hdf5:DataFromFile)"/>
<xsl:text>
</xsl:text>
    </xsl:if>
    </xsl:if>
    <!-- output Label if requested and exists -->
    <xsl:if test="$LABEL=1">
    <xsl:if test="@Name='Column Label'">
      <xsl:value-of select="$INDENT"/>Column Label: <xsl:value-of select="normalize-space(hdf5:Data/hdf5:DataFromFile)"/>
<xsl:text>
</xsl:text>
    </xsl:if>
    </xsl:if>
  </xsl:template>

</xsl:stylesheet>
