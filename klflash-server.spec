Summary: GNU The is a license program
Name: klflash-server
Version: 1.0
Release: 1%{?ns_dist}
Source0: %{name}-%{version}.tar.gz
#NoSource:1
#patch0:
License: GPL
Group: Development/Tools
URL:http://www.zd-tech.com.cn/
Vendor:Center of Infomation System Architecture Research

BuildRoot:  %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Requires: klflash-client >= 1.0-1
%description
The GNU ui program reformats C code to any of a variety of
formatting standards, or you can define your own.
%prep
%setup -q -b 0
#%setup -q -b 1
#%patch0 -b .orig
%build
#./configure
#cd %dirback && make
#cd $RPM_BUILD_DIR/%{name}-%{version}/%dirui 
#%%configure
#make CFLAGS="$RPM_OPT_FLAGS"
qmake-qt4
#make -f mymakefile 
#cp  %{_builddir}/%{name}-%{version}/libpword.so /usr/local/lib -rf
make %{?_smp_mflags}
%install
#rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/local/bin
#mkdir -p $RPM_BUILD_ROOT%{_libdir}
#cp -r libpword.so $RPM_BUILD_ROOT%{_libdir}
cp -r klflash $RPM_BUILD_ROOT/usr/local/bin 
#%%makeinstall
#make install DESTDIR=$RPM_BUILD_ROOT 
%clean
rm -rf $RPM_BUILD_ROOT
%files
%defattr(-,root,root)
/usr/local/bin/klflash
#%{_libdir}/libpword.so
#%doc /usr/local/info/ui.info
#%doc %attr(0444,root,root) /usr/local/man/man1/ui.1
#%doc COPYING AUTHORS README NEWS
%pre
%post

%preun
%postun
#rm /usr/bin/license 
#rm /usr/lib64/libpword.so
%changelog
* Mon Dec 21 2014 lixiaochun xcli@zd-tech.com.cn 
- license-server-1.0-1
