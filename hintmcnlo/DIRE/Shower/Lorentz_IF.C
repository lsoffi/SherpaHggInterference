#include "DIRE/Shower/Lorentz_IF.H"

#include "DIRE/Shower/Shower.H"
#include "PHASIC++/Channels/CSS_Kinematics.H"
#include "ATOOLS/Org/Message.H"

using namespace DIRE;
using namespace PHASIC;
using namespace ATOOLS;

Lorentz_IF::Lorentz_IF(const Kernel_Key &k):
  Lorentz(k,1)
{
}

double Lorentz_IF::Jacobian(const Splitting &s) const
{
  if (s.m_clu&1) return 1.0;
  double fo=p_sk->PS()->GetXPDF(s.m_eta,s.m_t,m_fl[0],s.p_c->Beam()-1);
  double fn=p_sk->PS()->GetXPDF(s.m_eta/s.m_x,s.m_t,m_fl[1],s.p_c->Beam()-1);
  if (dabs(fo)<p_sk->PS()->PDFMin()) return 0.0; 
  return fn/fo;
}

double Lorentz_IF::PDFEstimate(const Splitting &s) const
{
  double fo=p_sk->PS()->GetXPDF
    (s.m_eta,Min(s.m_t1,s.m_Q2),m_fl[0],s.p_c->Beam()-1);
  double fn=p_sk->PS()->GetXPDF
    (s.m_eta,Min(s.m_t1,s.m_Q2),m_fl[1],s.p_c->Beam()-1);
  if (m_fl[1]==Flavour(kf_u).Bar()||m_fl[1]==Flavour(kf_d).Bar()) {
    double fo0=p_sk->PS()->GetXPDF(s.m_eta,s.m_t0,m_fl[0],s.p_c->Beam()-1);
    double fn0=p_sk->PS()->GetXPDF(0.2,s.m_t0,m_fl[1],s.p_c->Beam()-1);
    if (fo0 && dabs(fo0)<dabs(fo)) fo=fo0;
    if (dabs(fn0)>dabs(fn)) fn=fn0;
  }
  if (fo==0.0) return 0.0;
  return dabs(fn/fo)*pow(1.0e6,s.m_eta*s.m_t0/Min(s.m_t1,s.m_Q2));
}

int Lorentz_IF::Construct(Splitting &s,const int mode) const
{
  Parton *b(NULL);
  if (s.m_kin==0)
    for (size_t i(0);i<s.p_c->Ampl()->size();++i)
      if ((*s.p_c->Ampl())[i]->Beam()==3-s.p_c->Beam()) {
	b=(*s.p_c->Ampl())[i];
	break;
      }
  Kin_Args ff(s.m_y,s.m_x,s.m_phi,s.m_kin);
  if (ConstructIFDipole
      (s.m_mi2,s.m_mj2,s.m_mij2,s.m_mk2,b?p_ms->Mass2(b->Flav()):0.0,
       -s.p_c->Mom(),s.p_s->Mom(),b?-b->Mom():Vec4D(),ff)<0)
    return -1;
  ff.m_pi=-ff.m_pi;
  if (b && p_sk->PS()->RemnantTest(s.p_c,ff.m_pi)<0) return -1; 
  return Update(s,ff,mode);
}

bool Lorentz_IF::Cluster(Splitting &s,const int mode) const
{
  Kin_Args ff=ClusterIFDipole
    (s.m_mi2,s.m_mj2,s.m_mij2,s.m_mk2,0.0,
     -s.p_c->Mom(),s.p_n->Mom(),s.p_s->Mom(),Vec4D(),
     mode|(s.m_kin?4:0));
  if (ff.m_stat<0) return false;
  SetParams(s,ff);
  s.m_t=s.m_Q2*s.m_y*(1.0-s.m_x);
  s.m_z=s.m_x;
  return true;
}

bool Lorentz_IF::Compute(Splitting &s) const
{
  s.m_y=s.m_t/s.m_Q2/(1.0-s.m_z);
  s.m_x=s.m_z;
  if (s.m_mk2==0.0)
    return s.m_y>0.0 && s.m_y<1.0;
  double muk2(s.m_mk2*s.m_z/s.m_Q2);
  double zp((1.0-s.m_z)/(1.0-s.m_z+muk2));
  return s.m_y>0.0 && s.m_y<zp;
}
