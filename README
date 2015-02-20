FbTerm - a fast FrameBuffer based TERMinal emulator for linux


DESCRIPTION

  FbTerm is a fast terminal emulator for linux with frame buffer device or VESA video card. Features include:
  
  * mostly as fast as terminal of linux kernel while accelerated scrolling is enabled
  * select font with fontconfig and draw text with freetype2, same as Qt/Gtk+ based GUI apps
  * dynamicly create/destroy up to 10 windows initially running default shell
  * record scrollback history for every window
  * auto-detect current locale and convert text encoding, support double width scripts like Chinese, Japanese etc
  * switch between configurable additional text encodings with hot keys on the fly
  * copy/past selected text between windows with mouse when gpm server is running
  * change the orientation of screen display, a.k.a. screen rotation
  * lightweight input method framework with client-server architecture
  * background image for eye candy


  read man page doc/fbterm.1 for usage help.


INPUT METHOD

  Instead of adding input method directly in FbTerm, a client-server based input method framework is designed to do 
this work. FbTerm acts as a client, standalone IM program as a server, and they run in seperated processes.

  As a normal IM user, please visit FbTerm's homepage (http://code.google.com/p/fbterm/) to check current available IM 
programs. If you want to develope a new IM program for FbTerm, there is a IM example in im/ directory, which help you to 
understand IM architecture and provide some base sources to simplify the development.
