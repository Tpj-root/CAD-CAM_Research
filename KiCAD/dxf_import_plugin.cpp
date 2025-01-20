\pcbnew\import_gfx\dxf_import_plugin.cpp
#include "tinyspline_lib/tinysplinecpp.h"
 
void DXF_IMPORT_PLUGIN::insertSpline( int aWidth )
{
    unsigned imax = m_curr_entity.m_SplineControlPointList.size();
    if( imax < 2 )  // malformed spline
        return;
 
    // Use bezier curves, supported by pcbnew, to approximate the spline
	tinyspline::BSpline dxfspline( m_curr_entity.m_SplineControlPointList.size(),
                                   /* coord dim */ 2, m_curr_entity.m_SplineDegree );
    std::vector<double> ctrlp;
 
    for( unsigned ii = 0; ii < imax; ++ii )
    {
        ctrlp.push_back( m_curr_entity.m_SplineControlPointList[ii].m_x );
        ctrlp.push_back( m_curr_entity.m_SplineControlPointList[ii].m_y );
    }
 
	dxfspline.setCtrlp( ctrlp );
	dxfspline.setKnots( m_curr_entity.m_SplineKnotsList );
	tinyspline::BSpline beziers( dxfspline.toBeziers() );
 
    std::vector<double> coords = beziers.ctrlp();
 
    // Each Bezier curve uses 4 vertices (a start point, 2 control points and a end point).
    // So we can have more than one Bezier curve ( there are one curve each four vertices)
    for( unsigned ii = 0; ii < coords.size(); ii += 8 )
    {
        VECTOR2D start( mapX( coords[ii] ), mapY( coords[ii+1] ) );
        VECTOR2D bezierControl1( mapX( coords[ii+2] ), mapY( coords[ii+3] ) );
        VECTOR2D bezierControl2( mapX( coords[ii+4] ), mapY( coords[ii+5] ) );
        VECTOR2D end( mapX( coords[ii+6] ), mapY( coords[ii+7] ) );
        m_internalImporter.AddSpline( start, bezierControl1, bezierControl2, end , aWidth );
    }
}
 
\pcbnew\import_gfx\graphics_importer_buffer.h
void ImportTo( GRAPHICS_IMPORTER& aImporter ) const override
{
    aImporter.AddSpline( m_start, m_bezierControl1, m_bezierControl2, m_end, m_width );
}
 
\pcbnew\import_gfx\graphics_importer_pcbnew.cpp
void GRAPHICS_IMPORTER_PCBNEW::AddSpline( const VECTOR2D& aStart, const VECTOR2D& BezierControl1,
                const VECTOR2D& BezierControl2, const VECTOR2D& aEnd, double aWidth )
{
    unique_ptr<DRAWSEGMENT> spline( createDrawing() );
    spline->SetShape( S_CURVE );
    spline->SetLayer( GetLayer() );
    spline->SetWidth( MapLineWidth( aWidth ) );
    spline->SetStart( MapCoordinate( aStart ) );
    spline->SetBezControl1( MapCoordinate( BezierControl1 ) );
    spline->SetBezControl2( MapCoordinate( BezierControl2 ) );
    spline->SetEnd( MapCoordinate( aEnd ) );
    spline->RebuildBezierToSegmentsPointsList( aWidth );
 
    if( spline->Type() == PCB_MODULE_EDGE_T )
        static_cast<EDGE_MODULE*>( spline.get() )->SetLocalCoord();
 
    addItem( std::move( spline ) );
}
 
\pcbnew\class_drawsegment.cpp
void DRAWSEGMENT::RebuildBezierToSegmentsPointsList( int aMinSegLen )
{
    // Has meaning only for S_CURVE DRAW_SEGMENT shape
    if( m_Shape != S_CURVE )
    {
        m_BezierPoints.clear();
        return;
    }
    // Rebuild the m_BezierPoints vertex list that approximate the Bezier curve
    std::vector<wxPoint> ctrlPoints = { m_Start, m_BezierC1, m_BezierC2, m_End };
    BEZIER_POLY converter( ctrlPoints );
    converter.GetPoly( m_BezierPoints, aMinSegLen );
}
 
\common\bezier_curves.cpp
void BEZIER_POLY::GetPoly( std::vector<wxPoint>& aOutput, int aMinSegLen )
{
}


