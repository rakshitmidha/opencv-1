#include "precomp.hpp"

namespace cv
{
    namespace viz
    {
        template<typename _Tp> Vec<_Tp, 3>* vtkpoints_data(vtkSmartPointer<vtkPoints>& points);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// line widget implementation
cv::viz::LineWidget::LineWidget(const Point3f &pt1, const Point3f &pt2, const Color &color)
{   
    vtkSmartPointer<vtkLineSource> line = vtkSmartPointer<vtkLineSource>::New();
    line->SetPoint1 (pt1.x, pt1.y, pt1.z);
    line->SetPoint2 (pt2.x, pt2.y, pt2.z);
    line->Update ();

    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New ();
    mapper->SetInput(line->GetOutput ());

    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);

    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}

void cv::viz::LineWidget::setLineWidth(float line_width)
{
    vtkActor *actor = vtkActor::SafeDownCast(WidgetAccessor::getProp(*this));
    CV_Assert(actor);
    actor->GetProperty()->SetLineWidth(line_width);
}

float cv::viz::LineWidget::getLineWidth()
{
    vtkActor *actor = vtkActor::SafeDownCast(WidgetAccessor::getProp(*this));
    CV_Assert(actor);
    return actor->GetProperty()->GetLineWidth();
}

template<> cv::viz::LineWidget cv::viz::Widget::cast<cv::viz::LineWidget>()
{
    Widget3D widget = this->cast<Widget3D>();
    return static_cast<LineWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// plane widget implementation

struct cv::viz::PlaneWidget::SetSizeImpl
{
    template<typename _Tp>
    static vtkSmartPointer<vtkPolyData> setSize(const Vec<_Tp, 3> &center, vtkSmartPointer<vtkPolyData> poly_data, double size)
    {
        vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
        transform->PreMultiply();
        transform->Translate(center[0], center[1], center[2]);
        transform->Scale(size, size, size);
        transform->Translate(-center[0], -center[1], -center[2]);
        
        vtkSmartPointer<vtkTransformPolyDataFilter> transform_filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        transform_filter->SetInput(poly_data);
        transform_filter->SetTransform(transform);
        transform_filter->Update();
        
        return transform_filter->GetOutput();
    }
};

cv::viz::PlaneWidget::PlaneWidget(const Vec4f& coefs, double size, const Color &color)
{    
    vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New ();
    plane->SetNormal (coefs[0], coefs[1], coefs[2]);
    double norm = cv::norm(Vec3f(coefs.val));
    plane->Push (-coefs[3] / norm);
    
    Vec3d p_center;
    plane->GetOrigin(p_center.val);
    
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New ();
    mapper->SetInput(SetSizeImpl::setSize(p_center, plane->GetOutput(), size));
    
    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}

cv::viz::PlaneWidget::PlaneWidget(const Vec4f& coefs, const Point3f& pt, double size, const Color &color)
{
    vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New ();
    Point3f coefs3(coefs[0], coefs[1], coefs[2]);
    double norm_sqr = 1.0 / coefs3.dot (coefs3);
    plane->SetNormal(coefs[0], coefs[1], coefs[2]);

    double t = coefs3.dot(pt) + coefs[3];
    Vec3f p_center = pt - coefs3 * t * norm_sqr;
    plane->SetCenter (p_center[0], p_center[1], p_center[2]);
    
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New ();
    mapper->SetInput(SetSizeImpl::setSize(p_center, plane->GetOutput(), size));

    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}

template<> cv::viz::PlaneWidget cv::viz::Widget::cast<cv::viz::PlaneWidget>()
{
    Widget3D widget = this->cast<Widget3D>();
    return static_cast<PlaneWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// sphere widget implementation

cv::viz::SphereWidget::SphereWidget(const Point3f &center, float radius, int sphere_resolution, const Color &color)
{
    vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New ();
    sphere->SetRadius (radius);
    sphere->SetCenter (center.x, center.y, center.z);
    sphere->SetPhiResolution (sphere_resolution);
    sphere->SetThetaResolution (sphere_resolution);
    sphere->LatLongTessellationOff ();
    sphere->Update ();
    
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New ();
    mapper->SetInput(sphere->GetOutput ());

    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);

    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}

template<> cv::viz::SphereWidget cv::viz::Widget::cast<cv::viz::SphereWidget>()
{
    Widget3D widget = this->cast<Widget3D>();
    return static_cast<SphereWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// arrow widget implementation

cv::viz::ArrowWidget::ArrowWidget(const Point3f& pt1, const Point3f& pt2, double thickness, const Color &color)
{
    vtkSmartPointer<vtkArrowSource> arrowSource = vtkSmartPointer<vtkArrowSource>::New ();
    arrowSource->SetShaftRadius(thickness);
    // The thickness and radius of the tip are adjusted based on the thickness of the arrow
    arrowSource->SetTipRadius(thickness * 3.0);
    arrowSource->SetTipLength(thickness * 10.0);
    
    float startPoint[3], endPoint[3];
    startPoint[0] = pt1.x;
    startPoint[1] = pt1.y;
    startPoint[2] = pt1.z;
    endPoint[0] = pt2.x;
    endPoint[1] = pt2.y;
    endPoint[2] = pt2.z;
    float normalizedX[3], normalizedY[3], normalizedZ[3];
    
    // The X axis is a vector from start to end
    vtkMath::Subtract(endPoint, startPoint, normalizedX);
    float length = vtkMath::Norm(normalizedX);
    vtkMath::Normalize(normalizedX);

    // The Z axis is an arbitrary vecotr cross X
    float arbitrary[3];
    arbitrary[0] = vtkMath::Random(-10,10);
    arbitrary[1] = vtkMath::Random(-10,10);
    arbitrary[2] = vtkMath::Random(-10,10);
    vtkMath::Cross(normalizedX, arbitrary, normalizedZ);
    vtkMath::Normalize(normalizedZ);

    // The Y axis is Z cross X
    vtkMath::Cross(normalizedZ, normalizedX, normalizedY);
    vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();

    // Create the direction cosine matrix
    matrix->Identity();
    for (unsigned int i = 0; i < 3; i++)
    {
        matrix->SetElement(i, 0, normalizedX[i]);
        matrix->SetElement(i, 1, normalizedY[i]);
        matrix->SetElement(i, 2, normalizedZ[i]);
    }    

    // Apply the transforms
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->Translate(startPoint);
    transform->Concatenate(matrix);
    transform->Scale(length, length, length);

    // Transform the polydata
    vtkSmartPointer<vtkTransformPolyDataFilter> transformPD = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformPD->SetTransform(transform);
    transformPD->SetInputConnection(arrowSource->GetOutputPort());
    
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New ();
    mapper->SetInput(transformPD->GetOutput ());

    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}

template<> cv::viz::ArrowWidget cv::viz::Widget::cast<cv::viz::ArrowWidget>()
{
    Widget3D widget = this->cast<Widget3D>();
    return static_cast<ArrowWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// circle widget implementation

cv::viz::CircleWidget::CircleWidget(const Point3f& pt, double radius, double thickness, const Color& color)
{
    vtkSmartPointer<vtkDiskSource> disk = vtkSmartPointer<vtkDiskSource>::New ();
    // Maybe the resolution should be lower e.g. 50 or 25
    disk->SetCircumferentialResolution (50);
    disk->SetInnerRadius (radius - thickness);
    disk->SetOuterRadius (radius + thickness);

    // Set the circle origin
    vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New ();
    t->Identity ();
    t->Translate (pt.x, pt.y, pt.z);

    vtkSmartPointer<vtkTransformPolyDataFilter> tf = vtkSmartPointer<vtkTransformPolyDataFilter>::New ();
    tf->SetTransform (t);
    tf->SetInputConnection (disk->GetOutputPort ());
    
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New ();
    mapper->SetInput(tf->GetOutput ());

    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}

template<> cv::viz::CircleWidget cv::viz::Widget::cast<cv::viz::CircleWidget>()
{
    Widget3D widget = this->cast<Widget3D>();
    return static_cast<CircleWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// cylinder widget implementation

cv::viz::CylinderWidget::CylinderWidget(const Point3f& pt_on_axis, const Point3f& axis_direction, double radius, int numsides, const Color &color)
{   
    const Point3f pt2 = pt_on_axis + axis_direction;
    vtkSmartPointer<vtkLineSource> line = vtkSmartPointer<vtkLineSource>::New ();
    line->SetPoint1 (pt_on_axis.x, pt_on_axis.y, pt_on_axis.z);
    line->SetPoint2 (pt2.x, pt2.y, pt2.z);
    
    vtkSmartPointer<vtkTubeFilter> tuber = vtkSmartPointer<vtkTubeFilter>::New ();
    tuber->SetInputConnection (line->GetOutputPort ());
    tuber->SetRadius (radius);
    tuber->SetNumberOfSides (numsides);
    
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New ();
    mapper->SetInput(tuber->GetOutput ());

    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New ();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}

template<> cv::viz::CylinderWidget cv::viz::Widget::cast<cv::viz::CylinderWidget>()
{
    Widget3D widget = this->cast<Widget3D>();
    return static_cast<CylinderWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// cylinder widget implementation

cv::viz::CubeWidget::CubeWidget(const Point3f& pt_min, const Point3f& pt_max, bool wire_frame, const Color &color)
{   
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New ();   
    if (wire_frame)
    {
        vtkSmartPointer<vtkOutlineSource> cube = vtkSmartPointer<vtkOutlineSource>::New();
        cube->SetBounds (pt_min.x, pt_max.x, pt_min.y, pt_max.y, pt_min.z, pt_max.z);
        mapper->SetInput(cube->GetOutput ());
    }
    else
    {
        vtkSmartPointer<vtkCubeSource> cube = vtkSmartPointer<vtkCubeSource>::New ();
        cube->SetBounds (pt_min.x, pt_max.x, pt_min.y, pt_max.y, pt_min.z, pt_max.z);
        mapper->SetInput(cube->GetOutput ());
    }
    
    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}

template<> cv::viz::CubeWidget cv::viz::Widget::cast<cv::viz::CubeWidget>()
{
    Widget3D widget = this->cast<Widget3D>();
    return static_cast<CubeWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// coordinate system widget implementation

cv::viz::CoordinateSystemWidget::CoordinateSystemWidget(double scale)
{
    vtkSmartPointer<vtkAxes> axes = vtkSmartPointer<vtkAxes>::New ();
    axes->SetOrigin (0, 0, 0);
    axes->SetScaleFactor (scale);

    vtkSmartPointer<vtkFloatArray> axes_colors = vtkSmartPointer<vtkFloatArray>::New ();
    axes_colors->Allocate (6);
    axes_colors->InsertNextValue (0.0);
    axes_colors->InsertNextValue (0.0);
    axes_colors->InsertNextValue (0.5);
    axes_colors->InsertNextValue (0.5);
    axes_colors->InsertNextValue (1.0);
    axes_colors->InsertNextValue (1.0);

    vtkSmartPointer<vtkPolyData> axes_data = axes->GetOutput ();
    axes_data->Update ();
    axes_data->GetPointData ()->SetScalars (axes_colors);

    vtkSmartPointer<vtkTubeFilter> axes_tubes = vtkSmartPointer<vtkTubeFilter>::New ();
    axes_tubes->SetInput (axes_data);
    axes_tubes->SetRadius (axes->GetScaleFactor () / 50.0);
    axes_tubes->SetNumberOfSides (6);
    
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New ();
    mapper->SetScalarModeToUsePointData ();
    mapper->SetInput(axes_tubes->GetOutput ());

    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
}

template<> cv::viz::CoordinateSystemWidget cv::viz::Widget::cast<cv::viz::CoordinateSystemWidget>()
{
    Widget3D widget = this->cast<Widget3D>();
    return static_cast<CoordinateSystemWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// polyline widget implementation

struct cv::viz::PolyLineWidget::CopyImpl
{    
    template<typename _Tp>
    static void copy(const Mat& source, Vec<_Tp, 3> *output, vtkSmartPointer<vtkPolyLine> polyLine)
    {
        int s_chs = source.channels();

        for(int y = 0, id = 0; y < source.rows; ++y)
        {
            const _Tp* srow = source.ptr<_Tp>(y);

            for(int x = 0; x < source.cols; ++x, srow += s_chs, ++id)
            {
                *output++ = Vec<_Tp, 3>(srow);
                polyLine->GetPointIds()->SetId(id,id);
            }
        }
    }
};

cv::viz::PolyLineWidget::PolyLineWidget(InputArray _pointData, const Color &color)
{
    Mat pointData = _pointData.getMat();
    CV_Assert(pointData.type() == CV_32FC3 || pointData.type() == CV_32FC4 || pointData.type() == CV_64FC3 || pointData.type() == CV_64FC4);
    vtkIdType nr_points = pointData.total();    
    
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New ();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New ();
    vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New ();
    
    if (pointData.depth() == CV_32F)
        points->SetDataTypeToFloat();
    else
        points->SetDataTypeToDouble();
    
    points->SetNumberOfPoints(nr_points);
    polyLine->GetPointIds()->SetNumberOfIds(nr_points);
    
    if (pointData.depth() == CV_32F)
    {
        // Get a pointer to the beginning of the data array
        Vec3f *data_beg = vtkpoints_data<float>(points);
        CopyImpl::copy(pointData, data_beg, polyLine);
    }
    else if (pointData.depth() == CV_64F)
    {
        // Get a pointer to the beginning of the data array
        Vec3d *data_beg = vtkpoints_data<double>(points);
        CopyImpl::copy(pointData, data_beg, polyLine);
    }
    
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    cells->InsertNextCell(polyLine);
    
    polyData->SetPoints(points);
    polyData->SetLines(cells);
    
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInput(polyData);
    
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}

template<> cv::viz::PolyLineWidget cv::viz::Widget::cast<cv::viz::PolyLineWidget>()
{
    Widget3D widget = this->cast<Widget3D>();
    return static_cast<PolyLineWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// grid widget implementation

cv::viz::GridWidget::GridWidget(Vec2i dimensions, Vec2d spacing, const Color &color)
{
    // Create the grid using image data
    vtkSmartPointer<vtkImageData> grid = vtkSmartPointer<vtkImageData>::New();
    
    // Add 1 to dimensions because in ImageData dimensions is the number of lines
    // - however here it means number of cells
    grid->SetDimensions(dimensions[0]+1, dimensions[1]+1, 1);
    grid->SetSpacing(spacing[0], spacing[1], 0.);
    
    // Set origin of the grid to be the middle of the grid
    grid->SetOrigin(dimensions[0] * spacing[0] * (-0.5), dimensions[1] * spacing[1] * (-0.5), 0);
    
    // Extract the edges so we have the grid
    vtkSmartPointer<vtkExtractEdges> filter = vtkSmartPointer<vtkExtractEdges>::New();
    filter->SetInputConnection(grid->GetProducerPort());
    filter->Update();
    
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInput(filter->GetOutput());
    
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}

template<> cv::viz::GridWidget cv::viz::Widget::cast<cv::viz::GridWidget>()
{
    Widget3D widget = this->cast<Widget3D>();
    return static_cast<GridWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// text3D widget implementation

cv::viz::Text3DWidget::Text3DWidget(const String &text, const Point3f &position, double text_scale, const Color &color)
{
    vtkSmartPointer<vtkVectorText> textSource = vtkSmartPointer<vtkVectorText>::New ();
    textSource->SetText (text.c_str());
    textSource->Update ();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New ();
    mapper->SetInputConnection (textSource->GetOutputPort ());
    
    vtkSmartPointer<vtkFollower> actor = vtkSmartPointer<vtkFollower>::New ();
    actor->SetMapper (mapper);
    actor->SetPosition (position.x, position.y, position.z);
    actor->SetScale (text_scale);
    
    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}

void cv::viz::Text3DWidget::setText(const String &text)
{
    vtkFollower *actor = vtkFollower::SafeDownCast(WidgetAccessor::getProp(*this));
    CV_Assert(actor);
    
    // Update text source
    vtkPolyDataMapper *mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
    vtkVectorText * textSource = vtkVectorText::SafeDownCast(mapper->GetInputConnection(0,0)->GetProducer());
    CV_Assert(textSource);
    
    textSource->SetText(text.c_str());
    textSource->Update();
}

cv::String cv::viz::Text3DWidget::getText() const
{
    vtkFollower *actor = vtkFollower::SafeDownCast(WidgetAccessor::getProp(*this));
    CV_Assert(actor);
    
    vtkPolyDataMapper *mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
    vtkVectorText * textSource = vtkVectorText::SafeDownCast(mapper->GetInputConnection(0,0)->GetProducer());
    CV_Assert(textSource);
    
    return textSource->GetText();
}

template<> cv::viz::Text3DWidget cv::viz::Widget::cast<cv::viz::Text3DWidget>()
{
    Widget3D widget = this->cast<Widget3D>();
    return static_cast<Text3DWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// text widget implementation

cv::viz::TextWidget::TextWidget(const String &text, const Point2i &pos, int font_size, const Color &color)
{
    vtkSmartPointer<vtkTextActor> actor = vtkSmartPointer<vtkTextActor>::New();
    actor->SetPosition (pos.x, pos.y);
    actor->SetInput (text.c_str ());

    vtkSmartPointer<vtkTextProperty> tprop = actor->GetTextProperty ();
    tprop->SetFontSize (font_size);
    tprop->SetFontFamilyToArial ();
    tprop->SetJustificationToLeft ();
    tprop->BoldOn ();

    Color c = vtkcolor(color);
    tprop->SetColor (c.val);
    
    WidgetAccessor::setProp(*this, actor);
}

template<> cv::viz::TextWidget cv::viz::Widget::cast<cv::viz::TextWidget>()
{
    Widget2D widget = this->cast<Widget2D>();
    return static_cast<TextWidget&>(widget);
}

void cv::viz::TextWidget::setText(const String &text)
{
    vtkTextActor *actor = vtkTextActor::SafeDownCast(WidgetAccessor::getProp(*this));
    CV_Assert(actor);
    actor->SetInput(text.c_str());
}

cv::String cv::viz::TextWidget::getText() const
{
    vtkTextActor *actor = vtkTextActor::SafeDownCast(WidgetAccessor::getProp(*this));
    CV_Assert(actor);
    return actor->GetInput();
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// image overlay widget implementation

struct cv::viz::ImageOverlayWidget::CopyImpl
{
    struct Impl
    {
        static void copyImageMultiChannel(const Mat &image, vtkSmartPointer<vtkImageData> output)
        {
            int i_chs = image.channels();
    
            for (int i = 0; i < image.rows; ++i)
            {
                const unsigned char * irows = image.ptr<unsigned char>(i);
                for (int j = 0; j < image.cols; ++j, irows += i_chs)
                {
                    unsigned char * vrows = static_cast<unsigned char *>(output->GetScalarPointer(j,i,0));
                    memcpy(vrows, irows, i_chs);
                    std::swap(vrows[0], vrows[2]); // BGR -> RGB
                }
            }
            output->Modified();
        }
        
        static void copyImageSingleChannel(const Mat &image, vtkSmartPointer<vtkImageData> output)
        {
            for (int i = 0; i < image.rows; ++i)
            {
                const unsigned char * irows = image.ptr<unsigned char>(i);
                for (int j = 0; j < image.cols; ++j, ++irows)
                {
                    unsigned char * vrows = static_cast<unsigned char *>(output->GetScalarPointer(j,i,0));
                    *vrows = *irows;
                }
            }
            output->Modified();
        }
    };
    
    static void copyImage(const Mat &image, vtkSmartPointer<vtkImageData> output)
    {
        int i_chs = image.channels();
        if (i_chs > 1)
        {
            // Multi channel images are handled differently because of BGR <-> RGB
            Impl::copyImageMultiChannel(image, output);
        }
        else
        {
            Impl::copyImageSingleChannel(image, output);
        }
    }
};

cv::viz::ImageOverlayWidget::ImageOverlayWidget(const Mat &image, const Rect &rect)
{
    CV_Assert(!image.empty() && image.depth() == CV_8U);
    
    // Create the vtk image and set its parameters based on input image
    vtkSmartPointer<vtkImageData> vtk_image = vtkSmartPointer<vtkImageData>::New();
    vtk_image->SetDimensions(image.cols, image.rows, 1);
    vtk_image->SetNumberOfScalarComponents(image.channels());
    vtk_image->SetScalarTypeToUnsignedChar();
    vtk_image->AllocateScalars();
    
    CopyImpl::copyImage(image, vtk_image);
    
    // Need to flip the image as the coordinates are different in OpenCV and VTK
    vtkSmartPointer<vtkImageFlip> flipFilter = vtkSmartPointer<vtkImageFlip>::New();
    flipFilter->SetFilteredAxis(1); // Vertical flip
    flipFilter->SetInputConnection(vtk_image->GetProducerPort());
    flipFilter->Update();
    
    // Scale the image based on the Rect
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->Scale(double(image.cols)/rect.width,double(image.rows)/rect.height,1.0);
    
    vtkSmartPointer<vtkImageReslice> image_reslice = vtkSmartPointer<vtkImageReslice>::New();
    image_reslice->SetResliceTransform(transform);
    image_reslice->SetInputConnection(flipFilter->GetOutputPort());
    image_reslice->SetOutputDimensionality(2);
    image_reslice->InterpolateOn();
    image_reslice->AutoCropOutputOn(); 
    
    vtkSmartPointer<vtkImageMapper> imageMapper = vtkSmartPointer<vtkImageMapper>::New();
    imageMapper->SetInputConnection(image_reslice->GetOutputPort());
    imageMapper->SetColorWindow(255); // OpenCV color
    imageMapper->SetColorLevel(127.5);  
    
    vtkSmartPointer<vtkActor2D> actor = vtkSmartPointer<vtkActor2D>::New();
    actor->SetMapper(imageMapper);
    actor->SetPosition(rect.x, rect.y);
    
    WidgetAccessor::setProp(*this, actor);
}

void cv::viz::ImageOverlayWidget::setImage(const Mat &image)
{
    CV_Assert(!image.empty() && image.depth() == CV_8U);
    
    vtkActor2D *actor = vtkActor2D::SafeDownCast(WidgetAccessor::getProp(*this));
    CV_Assert(actor);
    
    vtkImageMapper *mapper = vtkImageMapper::SafeDownCast(actor->GetMapper());
    CV_Assert(mapper);
    
    // Create the vtk image and set its parameters based on input image
    vtkSmartPointer<vtkImageData> vtk_image = vtkSmartPointer<vtkImageData>::New();
    vtk_image->SetDimensions(image.cols, image.rows, 1);
    vtk_image->SetNumberOfScalarComponents(image.channels());
    vtk_image->SetScalarTypeToUnsignedChar();
    vtk_image->AllocateScalars();
    
    CopyImpl::copyImage(image, vtk_image);
    
    // Need to flip the image as the coordinates are different in OpenCV and VTK
    vtkSmartPointer<vtkImageFlip> flipFilter = vtkSmartPointer<vtkImageFlip>::New();
    flipFilter->SetFilteredAxis(1); // Vertical flip
    flipFilter->SetInputConnection(vtk_image->GetProducerPort());
    flipFilter->Update();
    
    mapper->SetInputConnection(flipFilter->GetOutputPort());
}

template<> cv::viz::ImageOverlayWidget cv::viz::Widget::cast<cv::viz::ImageOverlayWidget>()
{
    Widget2D widget = this->cast<Widget2D>();
    return static_cast<ImageOverlayWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// image 3D widget implementation

struct cv::viz::Image3DWidget::CopyImpl
{
    struct Impl
    {
        static void copyImageMultiChannel(const Mat &image, vtkSmartPointer<vtkImageData> output)
        {
            int i_chs = image.channels();
    
            for (int i = 0; i < image.rows; ++i)
            {
                const unsigned char * irows = image.ptr<unsigned char>(i);
                for (int j = 0; j < image.cols; ++j, irows += i_chs)
                {
                    unsigned char * vrows = static_cast<unsigned char *>(output->GetScalarPointer(j,i,0));
                    memcpy(vrows, irows, i_chs);
                    std::swap(vrows[0], vrows[2]); // BGR -> RGB
                }
            }
            output->Modified();
        }
        
        static void copyImageSingleChannel(const Mat &image, vtkSmartPointer<vtkImageData> output)
        {
            for (int i = 0; i < image.rows; ++i)
            {
                const unsigned char * irows = image.ptr<unsigned char>(i);
                for (int j = 0; j < image.cols; ++j, ++irows)
                {
                    unsigned char * vrows = static_cast<unsigned char *>(output->GetScalarPointer(j,i,0));
                    *vrows = *irows;
                }
            }
            output->Modified();
        }
    };
    
    static void copyImage(const Mat &image, vtkSmartPointer<vtkImageData> output)
    {
        int i_chs = image.channels();
        if (i_chs > 1)
        {
            // Multi channel images are handled differently because of BGR <-> RGB
            Impl::copyImageMultiChannel(image, output);
        }
        else
        {
            Impl::copyImageSingleChannel(image, output);
        }
    }
};

cv::viz::Image3DWidget::Image3DWidget(const Mat &image, const Size &size)
{
    CV_Assert(!image.empty() && image.depth() == CV_8U);
    
    // Create the vtk image and set its parameters based on input image
    vtkSmartPointer<vtkImageData> vtk_image = vtkSmartPointer<vtkImageData>::New();
    vtk_image->SetDimensions(image.cols, image.rows, 1);
    vtk_image->SetNumberOfScalarComponents(image.channels());
    vtk_image->SetScalarTypeToUnsignedChar();
    vtk_image->AllocateScalars();
    
    CopyImpl::copyImage(image, vtk_image);
    
    // Need to flip the image as the coordinates are different in OpenCV and VTK
    vtkSmartPointer<vtkImageFlip> flipFilter = vtkSmartPointer<vtkImageFlip>::New();
    flipFilter->SetFilteredAxis(1); // Vertical flip
    flipFilter->SetInputConnection(vtk_image->GetProducerPort());
    flipFilter->Update();
    
    Vec3d plane_center(size.width * 0.5, size.height * 0.5, 0.0);
    
    vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
    plane->SetCenter(plane_center[0], plane_center[1], plane_center[2]);
    plane->SetNormal(0.0, 0.0, 1.0);
    
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->PreMultiply();
    transform->Translate(plane_center[0], plane_center[1], plane_center[2]);
    transform->Scale(size.width, size.height, 1.0);
    transform->Translate(-plane_center[0], -plane_center[1], -plane_center[2]);
    
    vtkSmartPointer<vtkTransformPolyDataFilter> transform_filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transform_filter->SetTransform(transform);
    transform_filter->SetInputConnection(plane->GetOutputPort());
    transform_filter->Update();
    
    // Apply the texture
    vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
    texture->SetInputConnection(flipFilter->GetOutputPort());
    
    vtkSmartPointer<vtkTextureMapToPlane> texturePlane = vtkSmartPointer<vtkTextureMapToPlane>::New();
    texturePlane->SetInputConnection(transform_filter->GetOutputPort());
    
    vtkSmartPointer<vtkPolyDataMapper> planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    planeMapper->SetInputConnection(texturePlane->GetOutputPort());
    
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(planeMapper);
    actor->SetTexture(texture);
     
    WidgetAccessor::setProp(*this, actor);
}

cv::viz::Image3DWidget::Image3DWidget(const Vec3f &position, const Vec3f &normal, const Vec3f &up_vector, const Mat &image, const Size &size)
{
    CV_Assert(!image.empty() && image.depth() == CV_8U);
    
    // Create the vtk image and set its parameters based on input image
    vtkSmartPointer<vtkImageData> vtk_image = vtkSmartPointer<vtkImageData>::New();
    vtk_image->SetDimensions(image.cols, image.rows, 1);
    vtk_image->SetNumberOfScalarComponents(image.channels());
    vtk_image->SetScalarTypeToUnsignedChar();
    vtk_image->AllocateScalars();
    
    CopyImpl::copyImage(image, vtk_image);
    
    // Need to flip the image as the coordinates are different in OpenCV and VTK
    vtkSmartPointer<vtkImageFlip> flipFilter = vtkSmartPointer<vtkImageFlip>::New();
    flipFilter->SetFilteredAxis(1); // Vertical flip
    flipFilter->SetInputConnection(vtk_image->GetProducerPort());
    flipFilter->Update();
    
    vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
    plane->SetCenter(0.0, 0.0, 0.0);
    plane->SetNormal(0.0, 0.0, 1.0);    
    
    // Compute the transformation matrix for drawing the camera frame in a scene
    Vec3f u,v,n;
    n = normalize(normal);
    u = normalize(up_vector.cross(n));
    v = n.cross(u);
    
    vtkSmartPointer<vtkMatrix4x4> mat_trans = vtkSmartPointer<vtkMatrix4x4>::New();
    mat_trans->SetElement(0,0,u[0]);
    mat_trans->SetElement(0,1,u[1]);
    mat_trans->SetElement(0,2,u[2]);
    mat_trans->SetElement(1,0,v[0]);
    mat_trans->SetElement(1,1,v[1]);
    mat_trans->SetElement(1,2,v[2]);
    mat_trans->SetElement(2,0,n[0]);
    mat_trans->SetElement(2,1,n[1]);
    mat_trans->SetElement(2,2,n[2]);
    // Inverse rotation (orthogonal, so just take transpose)
    mat_trans->Transpose(); 
    // Then translate the coordinate frame to camera position
    mat_trans->SetElement(0,3,position[0]);
    mat_trans->SetElement(1,3,position[1]);
    mat_trans->SetElement(2,3,position[2]);
    mat_trans->SetElement(3,3,1);
    
    // Apply the texture
    vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
    texture->SetInputConnection(flipFilter->GetOutputPort());
    
    vtkSmartPointer<vtkTextureMapToPlane> texturePlane = vtkSmartPointer<vtkTextureMapToPlane>::New();
    texturePlane->SetInputConnection(plane->GetOutputPort());
    
    // Apply the transform after texture mapping
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->PreMultiply();
    transform->SetMatrix(mat_trans);
    transform->Scale(size.width, size.height, 1.0);
    
    vtkSmartPointer<vtkTransformPolyDataFilter> transform_filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transform_filter->SetTransform(transform);
    transform_filter->SetInputConnection(texturePlane->GetOutputPort());
    transform_filter->Update();
    
    vtkSmartPointer<vtkPolyDataMapper> planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    planeMapper->SetInputConnection(transform_filter->GetOutputPort());
    
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(planeMapper);
    actor->SetTexture(texture);
     
    WidgetAccessor::setProp(*this, actor);
}

void cv::viz::Image3DWidget::setImage(const Mat &image)
{
    CV_Assert(!image.empty() && image.depth() == CV_8U);
    
    vtkActor *actor = vtkActor::SafeDownCast(WidgetAccessor::getProp(*this));
    CV_Assert(actor);
    
    // Create the vtk image and set its parameters based on input image
    vtkSmartPointer<vtkImageData> vtk_image = vtkSmartPointer<vtkImageData>::New();
    vtk_image->SetDimensions(image.cols, image.rows, 1);
    vtk_image->SetNumberOfScalarComponents(image.channels());
    vtk_image->SetScalarTypeToUnsignedChar();
    vtk_image->AllocateScalars();
    
    CopyImpl::copyImage(image, vtk_image);
    
    // Need to flip the image as the coordinates are different in OpenCV and VTK
    vtkSmartPointer<vtkImageFlip> flipFilter = vtkSmartPointer<vtkImageFlip>::New();
    flipFilter->SetFilteredAxis(1); // Vertical flip
    flipFilter->SetInputConnection(vtk_image->GetProducerPort());
    flipFilter->Update();
    
    // Apply the texture
    vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
    texture->SetInputConnection(flipFilter->GetOutputPort());
    
    actor->SetTexture(texture);
}

template<> cv::viz::Image3DWidget cv::viz::Widget::cast<cv::viz::Image3DWidget>()
{
    Widget3D widget = this->cast<Widget3D>();
    return static_cast<Image3DWidget&>(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// camera position widget implementation

cv::viz::CameraPositionWidget::CameraPositionWidget(double scale)
{
    vtkSmartPointer<vtkAxes> axes = vtkSmartPointer<vtkAxes>::New ();
    axes->SetOrigin (0, 0, 0);
    axes->SetScaleFactor (scale);
    
    vtkSmartPointer<vtkFloatArray> axes_colors = vtkSmartPointer<vtkFloatArray>::New ();
    axes_colors->Allocate (6);
    axes_colors->InsertNextValue (0.0);
    axes_colors->InsertNextValue (0.0);
    axes_colors->InsertNextValue (0.5);
    axes_colors->InsertNextValue (0.5);
    axes_colors->InsertNextValue (1.0);
    axes_colors->InsertNextValue (1.0);

    vtkSmartPointer<vtkPolyData> axes_data = axes->GetOutput ();
    axes_data->Update ();
    axes_data->GetPointData ()->SetScalars (axes_colors);
    
    vtkSmartPointer<vtkTubeFilter> axes_tubes = vtkSmartPointer<vtkTubeFilter>::New ();
    axes_tubes->SetInput (axes_data);
    axes_tubes->SetRadius (axes->GetScaleFactor () / 50.0);
    axes_tubes->SetNumberOfSides (6);
    
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New ();
    mapper->SetScalarModeToUsePointData ();
    mapper->SetInput(axes_tubes->GetOutput ());

    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
}

cv::viz::CameraPositionWidget::CameraPositionWidget(const Vec3f &position, const Vec3f &look_at, const Vec3f &up_vector, double scale)
{
    vtkSmartPointer<vtkAxes> axes = vtkSmartPointer<vtkAxes>::New ();
    axes->SetOrigin (0, 0, 0);
    axes->SetScaleFactor (scale);
    
    // Compute the transformation matrix for drawing the camera frame in a scene
    Vec3f u,v,n;
    n = normalize(look_at - position);
    u = normalize(up_vector.cross(n));
    v = n.cross(u);
    
    vtkSmartPointer<vtkMatrix4x4> mat_trans = vtkSmartPointer<vtkMatrix4x4>::New();
    mat_trans->SetElement(0,0,u[0]);
    mat_trans->SetElement(0,1,u[1]);
    mat_trans->SetElement(0,2,u[2]);
    mat_trans->SetElement(1,0,v[0]);
    mat_trans->SetElement(1,1,v[1]);
    mat_trans->SetElement(1,2,v[2]);
    mat_trans->SetElement(2,0,n[0]);
    mat_trans->SetElement(2,1,n[1]);
    mat_trans->SetElement(2,2,n[2]);
    // Inverse rotation (orthogonal, so just take transpose)
    mat_trans->Transpose(); 
    // Then translate the coordinate frame to camera position
    mat_trans->SetElement(0,3,position[0]);
    mat_trans->SetElement(1,3,position[1]);
    mat_trans->SetElement(2,3,position[2]);
    mat_trans->SetElement(3,3,1);

    vtkSmartPointer<vtkFloatArray> axes_colors = vtkSmartPointer<vtkFloatArray>::New ();
    axes_colors->Allocate (6);
    axes_colors->InsertNextValue (0.0);
    axes_colors->InsertNextValue (0.0);
    axes_colors->InsertNextValue (0.5);
    axes_colors->InsertNextValue (0.5);
    axes_colors->InsertNextValue (1.0);
    axes_colors->InsertNextValue (1.0);

    vtkSmartPointer<vtkPolyData> axes_data = axes->GetOutput ();
    axes_data->Update ();
    axes_data->GetPointData ()->SetScalars (axes_colors);
    
    // Transform the default coordinate frame
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->PreMultiply();
    transform->SetMatrix(mat_trans);
    
    vtkSmartPointer<vtkTransformPolyDataFilter> filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    filter->SetInput(axes_data);
    filter->SetTransform(transform);
    filter->Update();
    
    vtkSmartPointer<vtkTubeFilter> axes_tubes = vtkSmartPointer<vtkTubeFilter>::New ();
    axes_tubes->SetInput (filter->GetOutput());
    axes_tubes->SetRadius (axes->GetScaleFactor () / 50.0);
    axes_tubes->SetNumberOfSides (6);
    
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New ();
    mapper->SetScalarModeToUsePointData ();
    mapper->SetInput(axes_tubes->GetOutput ());

    vtkSmartPointer<vtkLODActor> actor = vtkSmartPointer<vtkLODActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
}

cv::viz::CameraPositionWidget::CameraPositionWidget(const Matx33f &K, double scale, const Color &color)
{
    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
    float f_x = K(0,0);
    float f_y = K(1,1);
    float c_y = K(1,2);
    float aspect_ratio = f_y / f_x;
    // Assuming that this is an ideal camera (c_y and c_x are at the center of the image)
    float fovy = 2.0f * atan2(c_y,f_y) * 180 / CV_PI;
    
    camera->SetViewAngle(fovy);
    camera->SetPosition(0.0,0.0,0.0);
    camera->SetViewUp(0.0,1.0,0.0);
    camera->SetFocalPoint(0.0,0.0,1.0);
    camera->SetClippingRange(0.01, scale);
    
    double planesArray[24];
    camera->GetFrustumPlanes(aspect_ratio, planesArray);
    
    vtkSmartPointer<vtkPlanes> planes = vtkSmartPointer<vtkPlanes>::New();
    planes->SetFrustumPlanes(planesArray);
    
    vtkSmartPointer<vtkFrustumSource> frustumSource =
    vtkSmartPointer<vtkFrustumSource>::New();
    frustumSource->SetPlanes(planes);
    frustumSource->Update();

    vtkSmartPointer<vtkExtractEdges> filter = vtkSmartPointer<vtkExtractEdges>::New();
    filter->SetInput(frustumSource->GetOutput());
    filter->Update();
    
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInput(filter->GetOutput());
    
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}


cv::viz::CameraPositionWidget::CameraPositionWidget(const Vec2f &fov, double scale, const Color &color)
{
    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
    
    camera->SetViewAngle(fov[1] * 180 / CV_PI); // Vertical field of view
    camera->SetPosition(0.0,0.0,0.0);
    camera->SetViewUp(0.0,1.0,0.0);
    camera->SetFocalPoint(0.0,0.0,1.0);
    camera->SetClippingRange(0.01, scale);
    
    double planesArray[24];
    // Default aspect ratio = 1.0? fovx/fovy?
    camera->GetFrustumPlanes(1.0, planesArray);
    
    vtkSmartPointer<vtkPlanes> planes = vtkSmartPointer<vtkPlanes>::New();
    planes->SetFrustumPlanes(planesArray);
    
    vtkSmartPointer<vtkFrustumSource> frustumSource =
    vtkSmartPointer<vtkFrustumSource>::New();
    frustumSource->SetPlanes(planes);
    frustumSource->Update();

    // Extract the edges so we have the grid
    vtkSmartPointer<vtkExtractEdges> filter = vtkSmartPointer<vtkExtractEdges>::New();
    filter->SetInput(frustumSource->GetOutput());
    filter->Update();    
    
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInput(filter->GetOutput());
    
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/// trajectory widget implementation

struct cv::viz::TrajectoryWidget::ApplyPath
{
    static void applyPath(vtkSmartPointer<vtkPolyData> poly_data, vtkSmartPointer<vtkAppendPolyData> append_filter, const std::vector<Affine3f> &path)
    {
        vtkSmartPointer<vtkMatrix4x4> mat_trans = vtkSmartPointer<vtkMatrix4x4>::New();
        mat_trans->Identity();
        
        vtkIdType nr_points = path.size();
        
        for (vtkIdType i = 0; i < nr_points; ++i)
        {
            vtkSmartPointer<vtkPolyData> new_data = vtkSmartPointer<vtkPolyData>::New();
            new_data->DeepCopy(poly_data);
            
            // Transform the default coordinate frame
            vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
            transform->PreMultiply();
            vtkMatrix4x4::Multiply4x4(convertToVtkMatrix(path[i].matrix), mat_trans, mat_trans);        
            transform->SetMatrix(mat_trans);
            
            vtkSmartPointer<vtkTransformPolyDataFilter> filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
            filter->SetInput(new_data);
            filter->SetTransform(transform);
            filter->Update();
            
            append_filter->AddInputConnection(filter->GetOutputPort());
        }
    }
};

cv::viz::TrajectoryWidget::TrajectoryWidget(const std::vector<Affine3f> &path, const Color &color, bool show_frames, double scale)
{
    vtkIdType nr_points = path.size();    
    
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New ();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New ();
    vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New ();
    
    points->SetDataTypeToFloat();
    points->SetNumberOfPoints(nr_points);
    polyLine->GetPointIds()->SetNumberOfIds(nr_points);
    
    Vec3f last_pos(0.0f,0.0f,0.0f);
    Vec3f *data_beg = vtkpoints_data<float>(points);
    *data_beg = path[0] * last_pos;
    
    for (vtkIdType i = 0; i < nr_points; ++i)
    {
        last_pos = path[i] * last_pos;
        *data_beg++ = last_pos;
        polyLine->GetPointIds()->SetId(i,i);
    }
    
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    cells->InsertNextCell(polyLine);
    
    polyData->SetPoints(points);
    polyData->SetLines(cells);
    
    vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
    if (show_frames)
    {
        vtkSmartPointer<vtkAxes> axes = vtkSmartPointer<vtkAxes>::New();
        axes->SetOrigin (0, 0, 0);
        axes->SetScaleFactor (scale);
        
        vtkSmartPointer<vtkUnsignedCharArray> axes_colors = vtkSmartPointer<vtkUnsignedCharArray>::New ();
        axes_colors->SetNumberOfComponents(3);
        axes_colors->InsertNextTuple3(255,0,0);
        axes_colors->InsertNextTuple3(255,0,0);
        axes_colors->InsertNextTuple3(0,255,0);
        axes_colors->InsertNextTuple3(0,255,0);
        axes_colors->InsertNextTuple3(0,0,255);
        axes_colors->InsertNextTuple3(0,0,255);
        
        vtkSmartPointer<vtkPolyData> axes_data = axes->GetOutput ();
        axes_data->Update ();
        axes_data->GetPointData ()->SetScalars (axes_colors);
        
        vtkSmartPointer<vtkTubeFilter> axes_tubes = vtkSmartPointer<vtkTubeFilter>::New ();
        axes_tubes->SetInput (axes_data);
        axes_tubes->SetRadius (axes->GetScaleFactor() / 50.0);
        axes_tubes->SetNumberOfSides (6);
        axes_tubes->Update();
        
        ApplyPath::applyPath(axes_tubes->GetOutput(), appendFilter, path);
    }
    
    // Set the color for polyData
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents(3);
    
    // TODO Make this more efficient
    for (int i = 0; i < nr_points; ++i)
        colors->InsertNextTuple3(color[2], color[1], color[0]);
    
    polyData->GetPointData()->SetScalars(colors);
    
    appendFilter->AddInputConnection(polyData->GetProducerPort());
    
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetScalarModeToUsePointData ();
    mapper->SetInput(appendFilter->GetOutput());
    
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
}

cv::viz::TrajectoryWidget::TrajectoryWidget(const std::vector<Affine3f> &path, const Matx33f &K, double scale, const Color &color)
{   
    vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
    
    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
    float f_x = K(0,0);
    float f_y = K(1,1);
    float c_y = K(1,2);
    float aspect_ratio = f_y / f_x;
    // Assuming that this is an ideal camera (c_y and c_x are at the center of the image)
    float fovy = 2.0f * atan2(c_y,f_y) * 180 / CV_PI;
    
    camera->SetViewAngle(fovy);
    camera->SetPosition(0.0,0.0,0.0);
    camera->SetViewUp(0.0,1.0,0.0);
    camera->SetFocalPoint(0.0,0.0,1.0);
    camera->SetClippingRange(0.01, scale);
    
    double planesArray[24];
    camera->GetFrustumPlanes(aspect_ratio, planesArray);
    
    vtkSmartPointer<vtkPlanes> planes = vtkSmartPointer<vtkPlanes>::New();
    planes->SetFrustumPlanes(planesArray);
    
    vtkSmartPointer<vtkFrustumSource> frustumSource = vtkSmartPointer<vtkFrustumSource>::New();
    frustumSource->SetPlanes(planes);
    frustumSource->Update();

    // Extract the edges
    vtkSmartPointer<vtkExtractEdges> filter = vtkSmartPointer<vtkExtractEdges>::New();
    filter->SetInput(frustumSource->GetOutput());
    filter->Update();
    
    ApplyPath::applyPath(filter->GetOutput(), appendFilter, path);
    
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInput(appendFilter->GetOutput());
    
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    
    WidgetAccessor::setProp(*this, actor);
    setColor(color);
}
