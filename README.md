# blocking_torcs

This a modified version of an all in one package of TORCS specially conceived to make its behavior closer to an OpenAI Gym 
environment and make it easier to apply Reinforcement Learning (RL) to TORCS. The one and only modification stands in src/scr_server/scr_server.cpp . 

In the original version, the server used to 
wait 10 ms for the client control command. If it received a new action within 10 ms it would apply this action in the simulator 
and then sends the new server state to the client. Otherwise, the last action received would be applied again.

In this modified version, the server waits indefinitely for the client control command, as would do any OpenAI gym environment. Therefore, I called it blocking_torcs.

I jointly wrote a Python Class ```TorcsEnv``` actually inheriting from the Class ```gym.Env``` making it really easy to apply existing implementations of RL algorithms to this environment. Available at https://github.com/DamienLancry/gym_torcs .

## Getting Started:

### Requirements
- Requires plib 1.8.5, FreeGLUT or GLUT, be aware to compile plib with -fPIC
  on AMD64 if you run a 64 bit version of Linux. Be aware that maybe just
  1.8.5 works.

### Installation
```
git clone https://github.com/DamienLancry/blocking_torcs
cd blocking_torcs
./configure --prefix=$HOME/.torcs --exec-prefix=$HOME/.torcs
```
(use --help for showing the options, of interest might be
  --enable-debug and --disable-xrandr).
```
make
make install
make datainstall
```
To benefit from the text mode when training your agents, you need to modify the configuration file located in 
$HOME/.torcs/config/raceman/quickrace.xml or $HOME/.torcs/config/raceman/practice.xml . You can directly edit it to modify it, or you can navigate in torcs GUI by launching
```
torcs
```
and then going through Race -> Quick Race -> Configure Race -> Accept
There should be no drivers selected, and ten instances of scr_server in not selected. 
Select a driver, for example ''damned 1'' for testing as in the following section or ''scr_server 1'' for using https://github.com/DamienLancry/gym_torcs.

Finally you need to add torcs to your path:
```
export PATH=$HOME/.torcs/bin:~/bin:$PATH
```
If you do not want to do it everytime you want to use torcs, add it to your bashrc and source it:
```
echo 'export PATH=$HOME/.torcs/bin:~/bin:$PATH' >> ~/.bashrc
source ~/.bashrc 
```

### Testing 

To test that everything is fine:
```
torcs -r $HOME/.torcs/config/raceman/quickrace.xml
```

## Further information and Aknowledgments

https://arxiv.org/pdf/1304.1672.pdf
