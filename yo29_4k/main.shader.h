/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef MAIN_SHADER_H_
# define MAIN_SHADER_H_

const char *main_shader_glsl =
 "#version 130\n"
 "uniform float F[1];"
 "float v=F[0];"
 "uniform sampler2D S;"
 "vec3 f=vec3(0.,.01,1.);"
 "vec4 t(vec2 v)"
 "{"
   "return texture2D(S,(v+.5)/textureSize(S,0));"
 "}"
 "vec4 i,r;"
 "float e(vec3 v)"
 "{"
   "float f=v.y-400.;"
   "v/=100.;"
   "f=v.y-2.*t(v.xz/10.).x;"
   "vec2 m=floor(v.xz),c=m;"
   "float y=10.,d=10.;"
   "for(float n=-1.;n<=1.;n+=1.)"
     "for(float z=-1.;z<=1.;z+=1.)"
       "{"
         "vec2 x=m+vec2(n,z),e=x+.5,p=floor(x/10.);"
         "r=t(p);"
         "i=t(x);"
         "e+=(i.yz-.5)*r.x;"
         "float s=abs(e.x-v.x)+abs(e.y-v.z);"
         "if(s<y)"
           "d=y,y=s,c=x;"
         "else"
           " if(s<d)"
             "d=s;"
       "}"
   "float z=d-y;"
   "i=t(c);"
   "r=t(floor(c/10.));"
   "float n=.1+3.*r.y;"
   "f=min(f,max(v.y-i.x*n,(.3-z)*.5));"
   "return 100.*max(f,length(v)-200.);"
 "}"
 "vec3 x(vec3 v)"
 "{"
   "return normalize(vec3(e(v+f.yxx),e(v+f.xyx),e(v+f.xxy))-e(v));"
 "}"
 "float e(vec3 v,vec3 f,float m,float z)"
 "{"
   "float y=10.,c=m;"
   "for(int i=0;i<99;++i)"
     "{"
       "float n=e(v+f*m);"
       "m+=n*.7;"
       "if(n<.001*m||m>z)"
         "return m;"
       "if(n<y)"
         "y=n,c=m;"
     "}"
   "return c;"
 "}"
 "vec3 m,c,n,y;"
 "vec4 l=vec4(1.,1.,1.,.5);"
 "float z=10.;"
 "float s(vec3 v)"
 "{"
   "return mix(max(0.,dot(n,v)),max(0.,pow(dot(n,normalize(v-c)),z)*(z+8.)/24.),l.w);"
 "}"
 "vec3 p=normalize(vec3(1.,1.+sin(v*.1),1.));"
 "const float w=6.36e+06,d=6.38e+06;"
 "const int o=64,a=32;"
 "const float g=.76,u=g*g,h=8000.,C=1200.,b=10.;"
 "vec3 L=vec3(0.,-w,0.),q=vec3(5.8e-06,1.35e-05,3.31e-05),Z=vec3(2.1e-05);"
 "float Y(in vec3 v)"
 "{"
   "vec3 f=floor(v),c=fract(v);"
   "vec2 m=f.xy+vec2(37.,17.)*f.z+c.xy,i=textureLod(S,(m+.5)/textureSize(S,0),0.).yx;"
   "return mix(i.x,i.y,c.z);"
 "}"
 "float X(in vec3 f)"
 "{"
   "return.55*Y(f)+.225*Y(f*2.+v*.4)+.125*Y(f*3.99)+.0625*Y(f*8.9);"
 "}"
 "float W(vec3 v)"
 "{"
   "float f=X(v*.0002);"
   "f=smoothstep(.44,.64,f);"
   "f*=f*40.;"
   "return f;"
 "}"
 "vec2 V(vec3 f)"
 "{"
   "float n=max(0.,length(f-L)-w);"
   "vec2 m=vec2(exp(-n/h),exp(-n/C));"
   "const float i=5000.;"
   "if(i<n&&n<10000.)"
     "m.y+=W(f+vec3(23175.7,0.,-v*3000.))*max(0.,sin(3.1415*(n-i)/i));"
   "return m;"
 "}"
 "float V(in vec3 v,in vec3 f,in float m)"
 "{"
   "vec3 c=v-L;"
   "float n=dot(c,f),y=dot(c,c)-m*m,i=n*n-y;"
   "if(i<0.)"
     "return-1.;"
   "float z=sqrt(i),d=-n-z,x=-n+z;"
   "return d>=0.?d:x;"
 "}"
 "vec3 V(vec3 v,vec3 f,float m,vec3 z)"
 "{"
   "vec2 i=vec2(0.,0.);"
   "vec3 n=vec3(0.),c=vec3(0.);"
   "float y=m/float(o);"
   "for(int r=0;r<o;++r)"
     "{"
       "vec3 x=v+f*float(r)*y;"
       "vec2 s=V(x)*y;"
       "i+=s;"
       "float e=V(x,p,d);"
       "if(e>0.)"
         "{"
           "float Y=e/float(a);"
           "vec2 t=vec2(0.);"
           "for(int w=0;w<a;++w)"
             "{"
               "vec3 L=x+p*float(w)*Y;"
               "t+=V(L)*Y;"
             "}"
           "vec2 w=t+i;"
           "vec3 L=exp(-(q*w.x+Z*w.y));"
           "n+=L*s.x;"
           "c+=L*s.y;"
         "}"
       "else"
         " return vec3(0.);"
     "}"
   "float x=dot(f,p),e=1.+x*x;"
   "vec2 r=vec2(.0596831*e,.119366*(1.-u)*e/((2.+u)*pow(1.+u-2.*g*x,1.5)));"
   "return z*exp(-(q*i.x+Z*i.y))+b*(n*q*r.x+c*Z*r.y);"
 "}"
 "void main()"
 "{"
   "const vec2 v=vec2(640.,360.);"
   "vec2 f=gl_FragCoord.xy/v*2.-1.;"
   "f.x*=v.x/v.y;"
   "vec3 r=vec3(0.);"
   "m=vec3(-1.,4.,8.)*100.;"
   "c=-normalize(vec3(.7,-.17,.67));"
   "vec3 w=normalize(cross(normalize(vec3(0.,1.,0.)),c));"
   "c=mat3(w,normalize(cross(c,w)),c)*normalize(vec3(f,-1.));"
   "vec3 L=vec3(0.);"
   "const float l=50000.;"
   "float Y=e(m,c,0.,l);"
   "if(Y<l)"
     "{"
       "y=m+c*Y;"
       "n=x(y);"
       "float Z=mod(y.y,3.)/3.;"
       "z=10.+Z*90.;"
       "L=i.xyz;"
       "L*=s(p);"
     "}"
   "else"
     " Y=V(m,c,d);"
   "L=V(m,c,Y,L);"
   "gl_FragColor=vec4(pow(L,vec3(1./2.2)),0.);"
 "}";

#endif // MAIN_SHADER_H_
