sudo apt install git vim screen virtualenv virtualenvwrapper trash-cli monit

mkvirtualenv deep --python=/home/pi/.virtualenvs/deep/bin/python3
pip install grabserial

git config --global merge.ff false
git config --global credential.helper store
