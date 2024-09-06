/*Copyright 2008-2022 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <gui_fdtd_structdesign.h>

GOP_Block::GOP_Block(wxWindow *parent,SymLib *lib,EMGeometry_GL *engine)
    :GeomOP_Panel(parent,lib,engine)
{
    wxStaticText *title=new wxStaticText(this,wxID_ANY,"Block");
    
    sizer->Add(title);
    
    x1=new NamedSymCtrl(this,"x1: ",0.0);
    x2=new NamedSymCtrl(this,"x2: ",0.0);
    
    y1=new NamedSymCtrl(this,"y1: ",0.0);
    y2=new NamedSymCtrl(this,"y2: ",0.0);
    
    z1=new NamedSymCtrl(this,"z1: ",0.0);
    z2=new NamedSymCtrl(this,"z2: ",0.0);
    
    mat=new NamedSymCtrl(this,"Mat: ",0.0);
    
    x1->set_lib(lib); x2->set_lib(lib);
    y1->set_lib(lib); y2->set_lib(lib);
    z1->set_lib(lib); z2->set_lib(lib);
    
    sizer->Add(x1,wxSizerFlags().Expand());
    sizer->Add(x2,wxSizerFlags().Expand());
    sizer->Add(y1,wxSizerFlags().Expand());
    sizer->Add(y2,wxSizerFlags().Expand());
    sizer->Add(z1,wxSizerFlags().Expand());
    sizer->Add(z2,wxSizerFlags().Expand());
    
    sizer->Add(mat,wxSizerFlags().Expand());
    
    vao=engine->request_vao();
    vao->set_mesh("block",0);
    
    x1->Bind(EVT_NAMEDTXTCTRL,&GOP_Block::evt_geometry,this);
    x2->Bind(EVT_NAMEDTXTCTRL,&GOP_Block::evt_geometry,this);
    y1->Bind(EVT_NAMEDTXTCTRL,&GOP_Block::evt_geometry,this);
    y2->Bind(EVT_NAMEDTXTCTRL,&GOP_Block::evt_geometry,this);
    z1->Bind(EVT_NAMEDTXTCTRL,&GOP_Block::evt_geometry,this);
    z2->Bind(EVT_NAMEDTXTCTRL,&GOP_Block::evt_geometry,this);
}


void GOP_Block::collapse()
{
    x1->hide();
    x2->hide();
    y1->hide();
    y2->hide();
    z1->hide();
    z2->hide();
    
    mat->hide();
    
    Layout();
}


void GOP_Block::evt_geometry(wxCommandEvent &event)
{
    update_vao();

    event.Skip();
}


void GOP_Block::expand()
{
    x1->show();
    x2->show();
    y1->show();
    y2->show();
    z1->show();
    z2->show();
    
    mat->show();
    
    Layout();
}


std::string GOP_Block::get_lua()
{
    std::stringstream strm;
    
    strm<<"add_block(";
    strm<<"\""<<x1->get_text()<<"\",";
    strm<<"\""<<x2->get_text()<<"\",";
    strm<<"\""<<y1->get_text()<<"\",";
    strm<<"\""<<y2->get_text()<<"\",";
    strm<<"\""<<z1->get_text()<<"\",";
    strm<<"\""<<z2->get_text()<<"\",";
    strm<<mat->get_value_integer()<<")";
    
    return strm.str();
}


int GOP_Block::get_material() { return mat->get_value_integer(); }


void GOP_Block::set(std::vector<std::string> const &args)
{
    x1->set_expression(args[0]); x2->set_expression(args[1]);
    y1->set_expression(args[2]); y2->set_expression(args[3]);
    z1->set_expression(args[4]); z2->set_expression(args[5]);
    mat->set_expression(args[6]);
    
    update_vao();
}


void GOP_Block::update_vao()
{
    double dx1=x1->get_value();
    double dx2=x2->get_value();
    double dy1=y1->get_value();
    double dy2=y2->get_value();
    double dz1=z1->get_value();
    double dz2=z2->get_value();
    
    O=Vector3(dx1,dy1,dz1);
    A=Vector3(dx2-dx1,0,0);
    B=Vector3(0,dy2-dy1,0);
    C=Vector3(0,0,dz2-dz1);
    
    vao->set_matrix(O,A,B,C);
}

//###############
//   GOP_Cone
//###############

GOP_Cone::GOP_Cone(wxWindow *parent,SymLib *lib,EMGeometry_GL *engine)
    :GeomOP_Panel(parent,lib,engine)
{
    wxStaticText *title=new wxStaticText(this,wxID_ANY,"Cone");
    
    sizer->Add(title);
    
    Ox=new NamedSymCtrl(this,"Ox: ",0.0);
    Oy=new NamedSymCtrl(this,"Oy: ",0.0);
    Oz=new NamedSymCtrl(this,"Oz: ",0.0);
    
    Hx=new NamedSymCtrl(this,"Hx: ",0.0);
    Hy=new NamedSymCtrl(this,"Hy: ",0.0);
    Hz=new NamedSymCtrl(this,"Hz: ",0.0);
    
    radius=new NamedSymCtrl(this,"Radius: ",0.0);
    
    mat=new NamedSymCtrl(this,"Mat: ",0.0);
    
    Ox->set_lib(lib); Oy->set_lib(lib); Oz->set_lib(lib);
    Hx->set_lib(lib); Hy->set_lib(lib); Hz->set_lib(lib);
    radius->set_lib(lib);
    
    sizer->Add(Ox,wxSizerFlags().Expand());
    sizer->Add(Oy,wxSizerFlags().Expand());
    sizer->Add(Oz,wxSizerFlags().Expand());
    
    sizer->Add(Hx,wxSizerFlags().Expand());
    sizer->Add(Hy,wxSizerFlags().Expand());
    sizer->Add(Hz,wxSizerFlags().Expand());
    
    sizer->Add(radius,wxSizerFlags().Expand());
    
    sizer->Add(mat,wxSizerFlags().Expand());
    
    vao=engine->request_vao();
    vao->set_mesh("cone",36);
    
    Ox->Bind(EVT_NAMEDTXTCTRL,&GOP_Cone::evt_geometry,this);
    Oy->Bind(EVT_NAMEDTXTCTRL,&GOP_Cone::evt_geometry,this);
    Oz->Bind(EVT_NAMEDTXTCTRL,&GOP_Cone::evt_geometry,this);
    
    Hx->Bind(EVT_NAMEDTXTCTRL,&GOP_Cone::evt_geometry,this);
    Hy->Bind(EVT_NAMEDTXTCTRL,&GOP_Cone::evt_geometry,this);
    Hz->Bind(EVT_NAMEDTXTCTRL,&GOP_Cone::evt_geometry,this);
    
    radius->Bind(EVT_NAMEDTXTCTRL,&GOP_Cone::evt_geometry,this);
}


void GOP_Cone::collapse()
{
    Ox->hide();
    Oy->hide();
    Oz->hide();
    
    Hx->hide();
    Hy->hide();
    Hz->hide();
    
    radius->hide();
    mat->hide();
    
    Layout();
}


void GOP_Cone::evt_geometry(wxCommandEvent &event)
{
    update_vao();

    event.Skip();
}


void GOP_Cone::expand()
{
    Ox->show();
    Oy->show();
    Oz->show();
    
    Hx->show();
    Hy->show();
    Hz->show();
    
    radius->show();
    mat->show();
    
    Layout();
}


std::string GOP_Cone::get_lua()
{
    std::stringstream strm;
    
    strm<<"add_cone(";
    strm<<"\""<<Ox->get_text()<<"\",";
    strm<<"\""<<Oy->get_text()<<"\",";
    strm<<"\""<<Oz->get_text()<<"\",";
    strm<<"\""<<Hx->get_text()<<"\",";
    strm<<"\""<<Hy->get_text()<<"\",";
    strm<<"\""<<Hz->get_text()<<"\",";
    strm<<"\""<<radius->get_text()<<"\",";
    strm<<mat->get_value_integer()<<")";
    
    return strm.str();
}


int GOP_Cone::get_material() { return mat->get_value_integer(); }


void GOP_Cone::set(std::vector<std::string> const &args)
{
    Ox->set_expression(args[0]);
    Oy->set_expression(args[1]);
    Oz->set_expression(args[2]);
    
    Hx->set_expression(args[3]);
    Hy->set_expression(args[4]);
    Hz->set_expression(args[5]);
    
    radius->set_expression(args[6]);
    
    mat->set_expression(args[7]);
    
    update_vao();
}


void GOP_Cone::update_vao()
{
    O=Vector3(Ox->get_value(),
              Oy->get_value(),
              Oz->get_value());
    
    C=Vector3(Hx->get_value(),
              Hy->get_value(),
              Hz->get_value());
    
    
    double r,phi,th;
    
    C.get_spherical(r,th,phi);
    r=radius->get_value();
    
    using std::cos;
    using std::sin;
    
    A=Vector3(r*cos(phi)*sin(th+Pi/2.0),
              r*sin(phi)*sin(th+Pi/2.0),
              r*cos(th+Pi/2.0));
              
    B.crossprod(C,A);
    B.normalize();
    B=r*B;
              
    vao->set_matrix(O,A,B,C);
}

//######################
//   GOP_Conf_Coating
//######################

GOP_Conf_Coating::GOP_Conf_Coating(wxWindow *parent, SymLib *lib, EMGeometry_GL *engine)
    :GeomOP_Panel(parent, lib, engine)
{
    wxStaticText *title = new wxStaticText(this,wxID_ANY,"Conformal Coating");

    sizer->Add(title);

    thickness = new NamedSymCtrl(this, "Thickness: ", 0.0);
    delta = new NamedSymCtrl(this, "Delta: ", 0.0);
    origin_mat = new NamedSymCtrl(this, "Origin Mat: ", 0.0);
    mat = new NamedSymCtrl(this, "Mat: ", 0.0);

    thickness->set_lib(lib);
    delta->set_lib(lib);
    origin_mat->set_lib(lib);

    sizer->Add(thickness, wxSizerFlags().Expand());
    sizer->Add(delta, wxSizerFlags().Expand());
    sizer->Add(origin_mat, wxSizerFlags().Expand());
    sizer->Add(mat, wxSizerFlags().Expand());
}


void GOP_Conf_Coating::collapse()
{
    thickness->Hide();
    delta->Hide();
    origin_mat->Hide();
    mat->Hide();

    Layout();
}


void GOP_Conf_Coating::expand()
{
    thickness->Show();
    delta->Show();
    origin_mat->Show();
    mat->Show();

    Layout();
}


std::string GOP_Conf_Coating::get_lua()
{
    std::stringstream strm;
    
    strm<<"add_conformal_coating(";
    strm<<"\""<<thickness->get_text()<<"\",";
    strm<<"\""<<delta->get_text()<<"\",";
    strm<<origin_mat->get_value_integer()<<",";
    strm<<mat->get_value_integer()<<")";
    
    return strm.str();
}


int GOP_Conf_Coating::get_material() { return mat->get_value_integer(); }


void GOP_Conf_Coating::set(std::vector<std::string> const &args)
{
    thickness->set_expression(args[0]);
    delta->set_expression(args[1]);
    origin_mat->set_expression(args[2]);
    
    mat->set_expression(args[3]);
}

//####################
//    GOP_Cylinder
//####################

GOP_Cylinder::GOP_Cylinder(wxWindow *parent,SymLib *lib,EMGeometry_GL *engine)
    :GeomOP_Panel(parent,lib,engine)
{
    wxStaticText *title=new wxStaticText(this,wxID_ANY,"Cylinder");
    
    sizer->Add(title);
    
    Ox=new NamedSymCtrl(this,"Ox: ",0.0);
    Oy=new NamedSymCtrl(this,"Oy: ",0.0);
    Oz=new NamedSymCtrl(this,"Oz: ",0.0);
    
    Hx=new NamedSymCtrl(this,"Hx: ",0.0);
    Hy=new NamedSymCtrl(this,"Hy: ",0.0);
    Hz=new NamedSymCtrl(this,"Hz: ",0.0);
    
    radius=new NamedSymCtrl(this,"Radius: ",0.0);
    
    mat=new NamedSymCtrl(this,"Mat: ",0.0);
    
    Ox->set_lib(lib); Oy->set_lib(lib); Oz->set_lib(lib);
    Hx->set_lib(lib); Hy->set_lib(lib); Hz->set_lib(lib);
    radius->set_lib(lib);
    
    sizer->Add(Ox,wxSizerFlags().Expand());
    sizer->Add(Oy,wxSizerFlags().Expand());
    sizer->Add(Oz,wxSizerFlags().Expand());
    
    sizer->Add(Hx,wxSizerFlags().Expand());
    sizer->Add(Hy,wxSizerFlags().Expand());
    sizer->Add(Hz,wxSizerFlags().Expand());
    
    sizer->Add(radius,wxSizerFlags().Expand());
    
    sizer->Add(mat,wxSizerFlags().Expand());
    
    vao=engine->request_vao();
    vao->set_mesh("cylinder",36);
    
    Ox->Bind(EVT_NAMEDTXTCTRL,&GOP_Cylinder::evt_geometry,this);
    Oy->Bind(EVT_NAMEDTXTCTRL,&GOP_Cylinder::evt_geometry,this);
    Oz->Bind(EVT_NAMEDTXTCTRL,&GOP_Cylinder::evt_geometry,this);
    
    Hx->Bind(EVT_NAMEDTXTCTRL,&GOP_Cylinder::evt_geometry,this);
    Hy->Bind(EVT_NAMEDTXTCTRL,&GOP_Cylinder::evt_geometry,this);
    Hz->Bind(EVT_NAMEDTXTCTRL,&GOP_Cylinder::evt_geometry,this);
    
    radius->Bind(EVT_NAMEDTXTCTRL,&GOP_Cylinder::evt_geometry,this);
}


void GOP_Cylinder::collapse()
{
    Ox->hide();
    Oy->hide();
    Oz->hide();
    
    Hx->hide();
    Hy->hide();
    Hz->hide();
    
    radius->hide();
    mat->hide();
    
    Layout();
}


void GOP_Cylinder::evt_geometry(wxCommandEvent &event)
{
    update_vao();

    event.Skip();
}


void GOP_Cylinder::expand()
{
    Ox->show();
    Oy->show();
    Oz->show();
    
    Hx->show();
    Hy->show();
    Hz->show();
    
    radius->show();
    mat->show();
    
    Layout();
}


std::string GOP_Cylinder::get_lua()
{
    std::stringstream strm;
    
    strm<<"add_cylinder(";
    strm<<"\""<<Ox->get_text()<<"\",";
    strm<<"\""<<Oy->get_text()<<"\",";
    strm<<"\""<<Oz->get_text()<<"\",";
    strm<<"\""<<Hx->get_text()<<"\",";
    strm<<"\""<<Hy->get_text()<<"\",";
    strm<<"\""<<Hz->get_text()<<"\",";
    strm<<"\""<<radius->get_text()<<"\",";
    strm<<mat->get_value_integer()<<")";
    
    return strm.str();
}


int GOP_Cylinder::get_material() { return mat->get_value_integer(); }


void GOP_Cylinder::set(std::vector<std::string> const &args)
{
    Ox->set_expression(args[0]);
    Oy->set_expression(args[1]);
    Oz->set_expression(args[2]);
    
    Hx->set_expression(args[3]);
    Hy->set_expression(args[4]);
    Hz->set_expression(args[5]);
    
    radius->set_expression(args[6]);
    
    mat->set_expression(args[7]);
    
    update_vao();
}


void GOP_Cylinder::update_vao()
{
    O=Vector3(Ox->get_value(),
              Oy->get_value(),
              Oz->get_value());
    
    C=Vector3(Hx->get_value(),
              Hy->get_value(),
              Hz->get_value());
    
    
    double r,phi,th;
    
    C.get_spherical(r,th,phi);
    r=radius->get_value();
    
    using std::cos;
    using std::sin;
    
    A=Vector3(r*cos(phi)*sin(th+Pi/2.0),
              r*sin(phi)*sin(th+Pi/2.0),
              r*cos(th+Pi/2.0));
              
    B.crossprod(C,A);
    B.normalize();
    B=r*B;
              
    vao->set_matrix(O,A,B,C);
}

//###############
//  GOP_Layer
//###############

GOP_Layer::GOP_Layer(wxWindow *parent,SymLib *lib,EMGeometry_GL *engine)
    :GeomOP_Panel(parent,lib,engine)
{
    wxStaticText *title=new wxStaticText(this,wxID_ANY,"Layer");
    
    sizer->Add(title);
    
    wxBoxSizer *choice_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    wxString choices[]={"X","Y","Z"};
    
    orient_txt=new wxStaticText(this,wxID_ANY,"Orientation: ");
    orientation=new wxChoice(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,3,choices);
    
    orientation->SetSelection(2);
    
    choice_sizer->Add(orient_txt,wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL)); // to be replaced with CenterVertical
    choice_sizer->Add(orientation,wxSizerFlags(1));
    
    h1=new NamedSymCtrl(this,"h1: ",0.0);
    h2=new NamedSymCtrl(this,"h2: ",0.0);
    
    mat=new NamedSymCtrl(this,"Mat: ",0.0);
    
    h1->set_lib(lib); h2->set_lib(lib);
    
    sizer->Add(choice_sizer,wxSizerFlags().Expand());
    
    sizer->Add(h1,wxSizerFlags().Expand());
    sizer->Add(h2,wxSizerFlags().Expand());
    
    sizer->Add(mat,wxSizerFlags().Expand());
    
    vao=engine->request_vao();
    vao->set_mesh("block",0);
    
    orientation->Bind(wxEVT_CHOICE,&GOP_Layer::evt_geometry,this);
    h1->Bind(EVT_NAMEDTXTCTRL,&GOP_Layer::evt_geometry,this);
    h2->Bind(EVT_NAMEDTXTCTRL,&GOP_Layer::evt_geometry,this);
}


void GOP_Layer::collapse()
{
    orient_txt->Hide();
    orientation->Hide();
    h1->hide();
    h2->hide();
    
    mat->hide();
    
    Layout();
}


void GOP_Layer::evt_geometry(wxCommandEvent &event)
{
    update_vao();

    event.Skip();
}


void GOP_Layer::expand()
{
    orient_txt->Show();
    orientation->Show();
    h1->show();
    h2->show();
    
    mat->show();
    
    Layout();
}


std::string GOP_Layer::get_lua()
{
    int dir=orientation->GetSelection();
    
    std::stringstream strm;
    
    strm<<"add_layer(";
    
         if(dir==0) strm<<"\"X\"";
    else if(dir==1) strm<<"\"Y\"";
    else if(dir==2) strm<<"\"Z\"";
    
    strm<<",\""<<h1->get_text()<<"\",\""<<h2->get_text()<<"\","<<mat->get_value_integer()<<")";
    
    return strm.str();
}


int GOP_Layer::get_material() { return mat->get_value_integer(); }


void GOP_Layer::set(std::vector<std::string> const &args)
{
    std::string const &dir=args[0];
    
    orientation->SetSelection(2);
    
    if(dir=="X") orientation->SetSelection(0);
    else if(dir=="Y") orientation->SetSelection(1);
    
    h1->set_expression(args[1]);
    h2->set_expression(args[2]);
    
    mat->set_expression(args[3]);
    
    update_vao();
}


void GOP_Layer::update_world(double lx_,double ly_,double lz_)
{
    GeomOP_Panel::update_world(lx_,ly_,lz_);
    
    update_vao();
}


void GOP_Layer::update_vao()
{
    int dir=orientation->GetSelection();
    
    double dh1=h1->get_value();
    double dh2=h2->get_value();
    
         if(dir==0)
    {
        O=Vector3(dh1,0,0);
        A=Vector3(dh2-dh1,0,0);
        B=Vector3(0,ly,0);
        C=Vector3(0,0,lz);
    }
    else if(dir==1)
    {
        O=Vector3(0,dh1,0);
        A=Vector3(lx,0,0);
        B=Vector3(0,dh2-dh1,0);
        C=Vector3(0,0,lz);
    }
    else if(dir==2)
    {
        O=Vector3(0,0,dh1);
        A=Vector3(lx,0,0);
        B=Vector3(0,ly,0);
        C=Vector3(0,0,dh2-dh1);
    }
    
    vao->set_matrix(O,A,B,C);
}

//###############
//   GOP_Sphere
//###############

GOP_Sphere::GOP_Sphere(wxWindow *parent,SymLib *lib,EMGeometry_GL *engine)
    :GeomOP_Panel(parent,lib,engine)
{
    wxStaticText *title=new wxStaticText(this,wxID_ANY,"Sphere");
    
    sizer->Add(title);
    
    Ox=new NamedSymCtrl(this,"Ox: ",0.0);
    Oy=new NamedSymCtrl(this,"Oy: ",0.0);
    Oz=new NamedSymCtrl(this,"Oz: ",0.0);
    
    radius=new NamedSymCtrl(this,"Radius: ",0.0);
    
    mat=new NamedSymCtrl(this,"Mat: ",0.0);
    
    Ox->set_lib(lib);
    Oy->set_lib(lib);
    Oz->set_lib(lib);
    radius->set_lib(lib);
    
    sizer->Add(Ox,wxSizerFlags().Expand());
    sizer->Add(Oy,wxSizerFlags().Expand());
    sizer->Add(Oz,wxSizerFlags().Expand());
    
    sizer->Add(radius,wxSizerFlags().Expand());
    
    sizer->Add(mat,wxSizerFlags().Expand());
    
    vao=engine->request_vao();
    vao->set_mesh("sphere",9);
    
    Ox->Bind(EVT_NAMEDTXTCTRL,&GOP_Sphere::evt_geometry,this);
    Oy->Bind(EVT_NAMEDTXTCTRL,&GOP_Sphere::evt_geometry,this);
    Oz->Bind(EVT_NAMEDTXTCTRL,&GOP_Sphere::evt_geometry,this);
    
    radius->Bind(EVT_NAMEDTXTCTRL,&GOP_Sphere::evt_geometry,this);
}


void GOP_Sphere::collapse()
{
    Ox->hide();
    Oy->hide();
    Oz->hide();
    
    radius->hide();
    mat->hide();
    
    Layout();
}


void GOP_Sphere::evt_geometry(wxCommandEvent &event)
{
    update_vao();

    event.Skip();
}


void GOP_Sphere::expand()
{
    Ox->show();
    Oy->show();
    Oz->show();
    
    radius->show();
    mat->show();
    
    Layout();
}


std::string GOP_Sphere::get_lua()
{
    std::stringstream strm;
    
    strm<<"add_sphere(";
    strm<<"\""<<Ox->get_text()<<"\",";
    strm<<"\""<<Oy->get_text()<<"\",";
    strm<<"\""<<Oz->get_text()<<"\",";
    strm<<"\""<<radius->get_text()<<"\",";
    strm<<mat->get_value_integer()<<")";
    
    return strm.str();
}


int GOP_Sphere::get_material() { return mat->get_value_integer(); }


void GOP_Sphere::set(std::vector<std::string> const &args)
{
    Ox->set_expression(args[0]);
    Oy->set_expression(args[1]);
    Oz->set_expression(args[2]);
    
    radius->set_expression(args[3]);
    
    mat->set_expression(args[4]);
    
    update_vao();
}


void GOP_Sphere::update_vao()
{
    double r=radius->get_value();
    
    O=Vector3(Ox->get_value(),
              Oy->get_value(),
              Oz->get_value());
    
    A(r,0,0);
    B(0,r,0);
    C(0,0,r);
              
    vao->set_matrix(O,A,B,C);
}
