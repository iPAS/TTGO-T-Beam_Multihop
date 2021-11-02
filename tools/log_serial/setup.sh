sudo apt install git vim screen virtualenv virtualenvwrapper trash-cli monit lsof

mkvirtualenv deep --python=/home/pi/.virtualenvs/deep/bin/python3
pip install grabserial
pip install dropbox

git submodule init && git submodule update && git submodule status

git config --global merge.ff false
git config --global credential.helper store

