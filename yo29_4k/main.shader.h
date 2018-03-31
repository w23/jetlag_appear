/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef MAIN_SHADER_H_
# define MAIN_SHADER_H_

const char *main_shader_glsl =
 "#version 130\n"
 "uniform int F;"
 "uniform sampler2D S;"
 "float v=F/352800.;"
 "vec3 i=vec3(0.,.01,1.);"
 "vec4 f(vec2 v)"
 "{"
   "return texture2D(S,(v+.5)/textureSize(S,0));"
 "}"
 "const float y=6.36e+06,z=6.38e+06;"
 "const int x=128,m=16;"
 "const float s=.76,e=s*s,t=8000.,l=1200.;"
 "float r=10.;"
 "const vec3 w=vec3(0.,-y,0.),n=vec3(5.8e-06,1.35e-05,3.31e-05),d=vec3(2e-05),a=d*1.1;"
 "vec4 c,p;"
 "vec3 u;"
 "float g,o;"
 "float h(vec3 v)"
 "{"
   "float i=v.y-400.,z=length(v.xz);"
   "v/=100.;"
   "vec2 y=floor(v.xz),V=y;"
   "float L=10.,m=10.;"
   "for(float b=-1.;b<=1.;b+=1.)"
     "for(float r=-1.;r<=1.;r+=1.)"
       "{"
         "vec2 x=y+vec2(b,r),Z=x+.5,X=floor(x/10.);"
         "p=f(X);"
         "float e=dot(x,x);"
         "c=f(x);"
         "Z+=(c.yz-.5)*smoothstep(100.,200.,e)*p.x;"
         "float s=abs(Z.x-v.x)+abs(Z.y-v.z);"
         "if(s<L)"
           "m=L,L=s,V=x;"
         "else"
           " if(s<m)"
             "m=s;"
       "}"
   "float x=m-L,X=dot(V,V);"
   "c=f(V);"
   "p=f(V/10.);"
   "u=v-vec3(V.x,0.,V.y);"
   "o=step(.8,p.w);"
   "float s=smoothstep(10000.,1000.,z)*(1.-o),b=s*(1.+4.*smoothstep(.8,1.,p.x));"
   "g=(.3-x)*.5;"
   "i=min(v.y,max(v.y-c.x*b,g));"
   "return 100.*max(i,length(v)-100.);"
 "}"
 "vec3 C(vec3 v)"
 "{"
   "return normalize(vec3(h(v+i.yxx),h(v+i.xyx),h(v+i.xxy))-h(v));"
 "}"
 "float C(vec3 v,vec3 i,float f,float x)"
 "{"
   "float y=10.,m=f;"
   "for(int r=0;r<199;++r)"
     "{"
       "float s=h(v+i*f);"
       "f+=s*.37;"
       "if(s<.001*f||f>x)"
         "return f;"
       "if(s<y)"
         "y=s,m=f;"
     "}"
   "return m;"
 "}"
 "vec3 b,L,q,Z;"
 "float Y=.5,X=10.;"
 "float W(vec3 v)"
 "{"
   "return mix(max(0.,dot(q,v))/3.,max(0.,pow(dot(q,normalize(v-L)),X)*(X+8.)/24.),Y);"
 "}"
 "vec3 V=vec3(1.,.001,1.);"
 "float U(in vec3 v)"
 "{"
   "vec3 s=floor(v),V=fract(v);"
   "vec2 i=s.xy+vec2(37.,17.)*s.z+V.xy,m=textureLod(S,(i+.5)/textureSize(S,0),0.).yx;"
   "return mix(m.x,m.y,V.z);"
 "}"
 "float T(in vec3 v)"
 "{"
   "return.55*U(v)+.225*U(v*2.)+.125*U(v*3.99)+.0625*U(v*8.9);"
 "}"
 "float R(vec3 v)"
 "{"
   "float f=T(v*.0004);"
   "f=smoothstep(.44,.64,f);"
   "f*=f*40.;"
   "return f;"
 "}"
 "vec2 Q(vec3 x)"
 "{"
   "float i=max(0.,length(x-w)-y);"
   "vec2 m=vec2(exp(-i/t),exp(-i/l));"
   "const float f=5000.;"
   "if(f<i&&i<10000.)"
     "m.y+=step(length(x),50000.)*R(x+vec3(23175.7,0.,v*30.))*max(0.,sin(3.1415*(i-f)/f));"
   "return m;"
 "}"
 "float C(in vec3 v,in vec3 x,in float f)"
 "{"
   "vec3 m=v-w;"
   "float s=dot(m,x),V=dot(m,m)-f*f,z=s*s-V;"
   "if(z<0.)"
     "return-1.;"
   "float i=sqrt(z),y=-s-i,L=-s+i;"
   "return y>=0.?y:L;"
 "}"
 "vec3 Q(vec3 v,vec3 i,float f)"
 "{"
   "float s=f/float(m);"
   "vec2 r=vec2(0.);"
   "for(int x=0;x<m;++x)"
     "{"
       "vec3 y=v+i*float(x)*s;"
       "r+=Q(y)*s;"
     "}"
   "return exp(-(n*r.x+a*r.y));"
 "}"
 "vec3 Q(vec3 v,vec3 i,float f,vec3 y)"
 "{"
   "vec2 c=vec2(0.,0.);"
   "vec3 b=vec3(0.),L=vec3(0.);"
   "float X=f/float(x);"
   "for(int Z=0;Z<x;++Z)"
     "{"
       "vec3 g=v+i*float(Z)*X;"
       "vec2 p=Q(g)*X;"
       "c+=p;"
       "float t=C(g,V,z);"
       "if(t>0.)"
         "{"
           "float U=t/float(m);"
           "vec2 u=vec2(0.);"
           "for(int w=0;w<m;++w)"
             "{"
               "vec3 Y=g+V*float(w)*U;"
               "u+=Q(Y)*U;"
             "}"
           "vec2 Y=u+c;"
           "vec3 w=exp(-(n*Y.x+a*Y.y));"
           "b+=w*p.x;"
           "L+=w*p.y;"
         "}"
       "else"
         " return vec3(0.);"
     "}"
   "float U=dot(i,V),p=1.+U*U;"
   "vec2 Y=vec2(.0596831*p,.119366*(1.-e)*p/((2.+e)*pow(1.+e-2.*s*U,1.5)));"
   "return y*exp(-(n*c.x+a*c.y))+r*(b*n*Y.x+L*d*Y.y);"
 "}"
 "float M(float v)"
 "{"
   "return clamp(v,0.,1.);"
 "}"
 "void main()"
 "{"
   "vec2 s=vec2(1280.,720.),m=gl_FragCoord.xy/s*2.-1.;"
   "m.x*=s.x/s.y;"
   "v+=f(gl_FragCoord.xy+v*100.*vec2(17.,39.)).x;"
   "vec3 y=vec3(0.);"
   "b=vec3(mod(v*2.,10000.)-5000.,1000.+500.*sin(v/60.),mod(v*10.,10000.)-5000.);"
   "if(v<64.)"
     "r=10.*v/64.,y=vec3(10000.,15000.,-10000.);"
   "else"
     " if(v<128.)"
       "{"
         "b.y=1000.;"
         "float x=(v-64.)/64.;"
         "b=vec3(x*1000.,1500.,3000.);"
         "y=b+vec3(-300.,0.,-300.);"
         "y.y=10.;"
         "V.y+=.01*x*x*x;"
       "}"
     "else"
       " if(v<144.)"
         "V.y=.01;"
       "else"
         "{"
           "float x=(v-144.)/64.;"
           "b=vec3(mod(x*2.,10000.)-500.,1000.-200.*x,mod(x*10.,10000.)-5000.);"
           "y.y=4000.*x;"
           "V.y=.01+.5*x;"
           "r=10.+200.*max(0.,(v-196.)/16.);"
         "}"
   "V=normalize(V);"
   "L=normalize(b-y);"
   "vec3 x=normalize(cross(i.xzx,L));"
   "L=mat3(x,normalize(cross(L,x)),L)*normalize(vec3(m,-1.));"
   "vec3 e=vec3(0.);"
   "const float w=50000.;"
   "float p=C(b,L,0.,w);"
   "vec3 U=vec3(0.);"
   "if(p<w)"
     "{"
       "Z=b+L*p;"
       "q=C(Z);"
       "vec3 n=vec3(.3+.2*c.w)+.03*c.xyz;"
       "float u=1.-.3*M(.5*((10.-Z.y)/10.+(1.-g)));"
       "X=200.;"
       "if(o>0.)"
         "{"
           "float t=step(.1,g);"
           "n=.05*mix(vec3(.4,.9,.3)-vec3(.2*f(Z.xz).x),.8*vec3(.6,.8,.2),t);"
           "Y=.5-.5*(1.-t);"
           "X=10.*(1.-t);"
           "u=1.;"
         "}"
       "else"
         " if(Z.y<1.)"
           "n=vec3(.01+.04*step(g,.07))+vec3(.7)*step(.145,g),X=1.;"
         "else"
           ";"
       "n*=u;"
       "e+=n*Q(Z,V,C(Z,V,z))*r*W(V);"
       "vec3 t=vec3(-V.x,V.y,-V.z);"
       "e+=n*Q(Z,t,C(Z,t,z),vec3(0.))*W(t);"
       "e+=n*vec3(.07)*W(normalize(vec3(1.)));"
       "e+=U;"
     "}"
   "else"
     " p=C(b,L,z);"
   "e=Q(b,L,p,e);"
   "gl_FragColor=vec4(pow(e,vec3(1./2.2)),.5);"
 "}";

#endif // MAIN_SHADER_H_
