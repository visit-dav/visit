<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/root">
    <html>
    <header><title>VTK Trace Objects</title></header>
    <body>
    <xsl:for-each select="leak_set">
        <h2>Leaks?</h2>
        <table border="1">
        <table cellspacing="5">
        <xsl:for-each select="type_summary">
            <tr><td><b>[<xsl:value-of select="type"/>]</b></td><td><xsl:value-of select="count"/></td></tr>
        </xsl:for-each>
        </table>
        </table>
        <br/>
        <table border="1">
        <table cellspacing="5">
            <tr><td>Address:</td>
                <td>Type:</td>
                <td>Ref Count:</td>
            </tr>
        <xsl:for-each select="leak">
            <tr><td><b>[<xsl:value-of select="addy"/>]</b></td>
                <td><xsl:value-of select="type"/></td>
                <td><xsl:value-of select="ref_count"/></td>
            </tr>
        </xsl:for-each>
        </table>
        </table>
    </xsl:for-each>
    <br/>
    <h2>VTK Trace Objects</h2>
    <xsl:for-each select="trace_set">
        <xsl:for-each select="trace_object">
        <table border="1">
            <table>
                <tr><td><b>[<xsl:value-of select="addy"/>]</b></td><td><xsl:value-of select="type"/></td></tr>
        </table>
        <table>
            <tr><td><b>Events:</b></td></tr>
            <xsl:for-each select="events/event">
            <tr><td><xsl:value-of select="text()"/></td></tr>
            </xsl:for-each>
        </table>
        </table>
        <br/>
        </xsl:for-each>
    </xsl:for-each>
    </body>
    </html>
</xsl:template>

</xsl:stylesheet>
