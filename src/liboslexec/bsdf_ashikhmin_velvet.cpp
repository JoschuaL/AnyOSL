/*
Copyright (c) 2009 Sony Pictures Imageworks, et al.
All Rights Reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of Sony Pictures Imageworks nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <cmath>

#include "oslops.h"
#include "oslexec_pvt.h"

#ifdef OSL_NAMESPACE
namespace OSL_NAMESPACE {
#endif

namespace OSL {
namespace pvt {

class AshikhminVelvetClosure : public BSDFClosure {
    Vec3 m_N;
    float m_sigma;
    float m_R0;

public:
    CLOSURE_CTOR (AshikhminVelvetClosure) : BSDFClosure(side, Labels::DIFFUSE)
    {
        CLOSURE_FETCH_ARG (m_N, 1);
        CLOSURE_FETCH_ARG (m_sigma, 2);
        CLOSURE_FETCH_ARG (m_R0, 3);

        m_sigma = std::max(m_sigma, 0.01f);
    }

    void print_on (std::ostream &out) const
    {
        out << "ashikhmin_velvet (";
        out << "(" << m_N[0] << ", " << m_N[1] << ", " << m_N[2] << "), ";
        out << m_sigma << ", ";
        out << m_R0;
        out << ")";
    }

    Color3 eval_reflect (const Vec3 &omega_out, const Vec3 &omega_in, float normal_sign, float& pdf) const
    {
        float cosNO = normal_sign * m_N.dot(omega_out);
        float cosNI = normal_sign * m_N.dot(omega_in);
        if (cosNO > 0 && cosNI > 0) {
            Vec3 H = omega_in + omega_out;
            H.normalize();

            float cosNH = normal_sign * m_N.dot(H);
            float cosHO = fabs(omega_out.dot(H));

            float cosNHdivHO = cosNH / cosHO;     
            cosNHdivHO = std::max(cosNHdivHO, 0.00001f);

            float fac1 = 2 * fabs(cosNHdivHO * cosNO);
            float fac2 = 2 * fabs(cosNHdivHO * cosNI);

            float cotangent =  cosNH / sqrtf(std::max((1 - cosNH * cosNH), 0.0f)); 

            float D = expf(-(cotangent * cotangent) / m_sigma);
            float G = std::min(1.0f, std::min(fac1, fac2));
            // Schlick approximation of Fresnel reflectance
            float cosi2 = cosNO * cosNO;
            float cosi5 = cosi2 * cosi2 * cosNO;
            float F =  m_R0 + (1 - cosi5) * (1 - m_R0);

            float out = (D * G * F) / cosNO; 

            pdf = 0.5f * (float) M_1_PI;
            return Color3 (out, out, out);
        }
        return Color3 (0, 0, 0);
    }

    Color3 eval_transmit (const Vec3 &omega_out, const Vec3 &omega_in, float normal_sign, float& pdf) const
    {
        return Color3 (0, 0, 0);
    }

    ustring sample (const Vec3 &Ng,
                 const Vec3 &omega_out, const Vec3 &domega_out_dx, const Vec3 &domega_out_dy,
                 float randu, float randv,
                 Vec3 &omega_in, Vec3 &domega_in_dx, Vec3 &domega_in_dy,
                 float &pdf, Color3 &eval) const
    {

        Vec3 Ngf, Nf;
        if (faceforward (omega_out, Ng, m_N, Ngf, Nf)) {
            // we are viewing the surface from above - send a ray out with uniform
            // distribution over the hemisphere
            sample_uniform_hemisphere (Nf, omega_out, randu, randv, omega_in, pdf);
            if (Ngf.dot(omega_in) > 0) {           
                Vec3 H = omega_in + omega_out;
                H.normalize();

                float D, G, F;
                float cosNI = Nf.dot(omega_in);
                float cosNO = Nf.dot(omega_out);
                float cosNH = Nf.dot(H);
                float cosHO = fabs(omega_out.dot(H));

                float cosNHdivHO = cosNH/cosHO;     
                cosNHdivHO = std::max(cosNHdivHO, 0.00001f);

                float fac1 = 2.f * fabs(cosNHdivHO * cosNO);
                float fac2 = 2.f * fabs(cosNHdivHO * cosNI);
               
                float cotangent =  cosNH / sqrtf(std::max((1.f - cosNH*cosNH), 0.f)); 
        
                D = expf(-(cotangent*cotangent) / m_sigma);
                G = std::min(1.f, std::min(fac1, fac2));
                // Schlick approximation of Fresnel reflectance
                float cosi2 = cosNO * cosNO;
                float cosi5 = cosi2 * cosi2 * cosNO;
                F =  m_R0 + (1 - cosi5) * (1 - m_R0);
       
                float power = (D*G*F)/cosNO;            
           
                eval.setValue(power, power, power);

                // TODO: find a better approximation for the retroreflective bounce
                domega_in_dx = (2 * Nf.dot(domega_out_dx)) * Nf - domega_out_dx;
                domega_in_dy = (2 * Nf.dot(domega_out_dy)) * Nf - domega_out_dy;
                domega_in_dx *= 125;
                domega_in_dy *= 125;    
            } else
                pdf = 0;      
        }
        return Labels::REFLECT;
    }

};

DECLOP (OP_ashikhmin_velvet)
{
    closure_op_guts<AshikhminVelvetClosure, 4> (exec, nargs, args,
            runflags, beginpoint, endpoint);
}

}; // namespace pvt
}; // namespace OSL
#ifdef OSL_NAMESPACE
}; // end namespace OSL_NAMESPACE
#endif
