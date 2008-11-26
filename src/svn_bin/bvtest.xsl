<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/bvtest">
    <html>
    <header><title>bvtest.py results</title></header>
    <link rel="stylesheet" type="text/css" href="bvtest.css" title="Style"/>
    <body>
        <table class="wikitable">
        <tr>
            <td colspan="6">
            <b>test_build_visit.py results:</b>
            </td>
        </tr>
        
        <tr><td class="rh">Host</td> <td colspan="5"><xsl:value-of select="@host"/></td></tr>
        <tr><td class="rh">Libraries</td><td colspan="5"><xsl:value-of select="@libs"/></td></tr>
        <tr><td class="rh">Start</td> <td colspan="5"><xsl:value-of select="@start"/></td></tr>
        <tr><td class="rh">End</td> <td colspan="5"><xsl:value-of select="@end"/></td></tr>
        
    <xsl:for-each select="results">
        <tr class="rh">
            <td>Library</td>
            <td>Command</td>
            <td>Build Start</td>
            <td>Build End</td>
            <td>Result</td>
            <td>Log File</td>
        </tr>
        <xsl:for-each select="build_result">
            <xsl:choose>
            <xsl:when test="result='success'">
                <tr class="green">
                <td><b><xsl:value-of select="lib"/></b></td>
                <td><xsl:value-of select="cmd"/></td>
                <td><xsl:value-of select="start"/></td>
                <td><xsl:value-of select="end"/></td>
                <td><b><xsl:value-of select="result"/></b></td>
                <td>
                    <xsl:element name="a">
                    <xsl:attribute name="href">
                        <xsl:value-of select="log" />
                    </xsl:attribute>
                    [open]
                    </xsl:element>
                </td>
                </tr>
            </xsl:when>
            <xsl:otherwise>
                <tr class="red">
                <td><b><xsl:value-of select="lib"/></b></td>
                <td><xsl:value-of select="cmd"/></td>
                <td><xsl:value-of select="start"/></td>
                <td><xsl:value-of select="end"/></td>
                <td><b><xsl:value-of select="result"/></b></td>
                <td>
                    <xsl:element name="a">
                    <xsl:attribute name="href">
                        <xsl:value-of select="log" />
                    </xsl:attribute>
                    [open]
                    </xsl:element>
                </td>
                </tr>
            </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:for-each>
    </table>
    </body>
    </html>
</xsl:template>

</xsl:stylesheet>
