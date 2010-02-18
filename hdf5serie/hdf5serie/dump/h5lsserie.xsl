<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:hdf5="http://hdfgroup.org/DTDs/HDF5-File">

  <xsl:param name="DESCRIPTION"/>
  <xsl:param name="LABEL"/>
  <xsl:param name="FOLLOW"/>
  <xsl:param name="FILENAME"/>
  <xsl:param name="DIR"/>

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
    <!--<xsl:apply-templates select="hdf5:Attribute">
      <xsl:with-param name="INDENT" select="concat($INDENT,'  ')"/>
    </xsl:apply-templates>-->
    <!-- apply H5:Dataset; increase indent -->
    <!--<xsl:apply-templates select="hdf5:Dataset">
      <xsl:with-param name="INDENT" select="concat($INDENT,'  ')"/>
      <xsl:with-param name="PATH" select="concat($PATH,'/',@Name)"/>
    </xsl:apply-templates>-->
    <!-- apply H5:Group; increase indent -->
    <!--<xsl:apply-templates select="hdf5:Group">
      <xsl:with-param name="INDENT" select="concat($INDENT,'  ')"/>
      <xsl:with-param name="PATH" select="concat($PATH,'/',@Name)"/>
    </xsl:apply-templates>-->
    <!-- apply all with increased indent -->
    <xsl:apply-templates select="*">
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
    <!-- apply H5:DataType; increase indent -->
    <xsl:apply-templates select="hdf5:DataType">
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

  <!-- output H5:DataType (for Member Label) -->
  <xsl:template match="hdf5:DataType">
    <xsl:param name="INDENT"/>
    <xsl:apply-templates select="hdf5:CompoundType">
      <xsl:with-param name="INDENT" select="$INDENT"/>
    </xsl:apply-templates>
  </xsl:template>
  <!-- output H5:CompoundType -->
  <xsl:template match="hdf5:CompoundType">
    <xsl:param name="INDENT"/>
    <xsl:value-of select="$INDENT"/>Member Label: <xsl:text></xsl:text>
    <xsl:apply-templates select="hdf5:Field">
      <xsl:with-param name="INDENT" select="$INDENT"/>
    </xsl:apply-templates>
    <xsl:text>
</xsl:text>
  </xsl:template>
  <!-- output H5:Field -->
  <xsl:template match="hdf5:Field">
    <xsl:param name="INDENT"/>
    <xsl:text>"</xsl:text><xsl:value-of select="@FieldName"/>
    <xsl:if test="hdf5:DataType/hdf5:ArrayType/hdf5:ArrayDimension">
      <xsl:text>(</xsl:text>
      <xsl:value-of select="hdf5:DataType/hdf5:ArrayType/hdf5:ArrayDimension/@DimSize"/>
      <xsl:text>)</xsl:text>
    </xsl:if>
    <xsl:text>" </xsl:text>
  </xsl:template>

  <!-- output H5:ExternalLink -->
  <xsl:template match="hdf5:ExternalLink">
    <xsl:param name="INDENT"/>
    <xsl:param name="PATH"/>
    <xsl:value-of select="$INDENT"/>* <xsl:value-of select="@LinkName"/> (External Link: <xsl:value-of select="concat($DIR,@TargetFilename,@TargetPath)"/>)<xsl:text>
</xsl:text>
    <xsl:if test="$FOLLOW=1">
      <!-- apply linked file -->
      <xsl:apply-templates select="document(concat($DIR,'.',@TargetFilename,'.xml'))/hdf5:HDF5-File">
        <xsl:with-param name="INDENT" select="concat($INDENT,'  ')"/>
        <xsl:with-param name="PATH" select="concat($PATH,'/',@LinkName)"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <!-- output H5:SoftLink -->
  <xsl:template match="hdf5:SoftLink">
    <xsl:param name="INDENT"/>
    <xsl:param name="PATH"/>
    <xsl:value-of select="$INDENT"/>* <xsl:value-of select="@LinkName"/> (Soft Link: <xsl:value-of select="@TargetPath"/>)<xsl:text>
</xsl:text>
  </xsl:template>

</xsl:stylesheet>
