pushd libzt
rm -fv libzt.py node.py select.py sockets.py
popd

pushd ../../src/bindings/python
rm -fv zt_wrap.cpp zt_wrap.h libzt.py
popd

rm -rfv __pycache__
rm -rfv dist
rm -rfv build
rm -fv setup.py

