!! $XdotOrg: $
!!
!! Copyright (c) 2000 by Conectiva S.A. (http://www.conectiva.com)
!! 
!! Permission is hereby granted, free of charge, to any person obtaining a
!! copy of this software and associated documentation files (the "Software"),
!! to deal in the Software without restriction, including without limitation
!! the rights to use, copy, modify, merge, publish, distribute, sublicense,
!! and/or sell copies of the Software, and to permit persons to whom the
!! Software is furnished to do so, subject to the following conditions:
!! 
!! The above copyright notice and this permission notice shall be included in
!! all copies or substantial portions of the Software.
!!  
!! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
!! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
!! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
!! CONECTIVA LINUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
!! WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
!! OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
!! SOFTWARE.
!! 
!! Except as contained in this notice, the name of Conectiva Linux shall
!! not be used in advertising or otherwise to promote the sale, use or other
!! dealings in this Software without prior written authorization from
!! Conectiva Linux.
!!
!! Author: Paulo César Pereira de Andrade <pcpa@conectiva.com.br>
!!
!! $XFree86: xc/programs/Xserver/hw/xfree86/xf86cfg/XF86Cfg.ad,v 1.13 2001/05/15 18:22:23 paulo Exp $
!!

*Form.background:			gray85
*Label.background:			gray85
!! keyboard
*Core.background:			gray85
*Label.foreground:			gray20
*Command.background:			gray80
*Command.foreground:			gray20
*Command.borderWidth:			0
*Command.displayList:\
foreground	gray60;\
lines		1,-1,-1,-1,-1,1;\
foreground	gray90;\
lines		-1,0,0,0,0,-1

*Text.?.foreground:			gray20
*Text.borderWidth:			0
*Text.background:			gray96
*Text.?.cursorColor:			rgb:d/5/5
*Text.displayList:\
foreground	white;\
lines		1,-1,-1,-1,-1,1;\
foreground	gray40;\
lines		-1,0,0,0,0,-1

*baseTranslations: #override \
<Key>Escape:	vidmode-restore()

*List.background:			gray96
*List.foreground:			gray20
*Viewport.borderColor:			gray40
*List.borderColor:			gray40

*MenuButton.background:			gray80
*MenuButton.foreground:			gray20
*MenuButton.borderWidth:		0
*MenuButton.displayList:\
line-style	solid;\
foreground	gray40;\
lines		1,-1,-1,-1,-1,1;\
foreground	gray90;\
lines		-1,0,0,0,0,-1;\
line-style	onoffdash;\
foreground	gray80;\
draw-rect	1,1,-2,-2

*SimpleMenu.background:			gray80
*SimpleMenu.?.foreground:		gray20
*SimpleMenu.borderWidth:		0
*SimpleMenu.displayList:\
foreground	gray40;\
lines		1,-1,-1,-1,-1,1;\
foreground	gray90;\
lines		-1,0,0,0,0,-1

*Dialog.displayList:\
foreground	gray40;\
lines		1,-1,-1,-1,-1,1;\
foreground	gray90;\
lines		-1,0,0,0,0,-1

*Toggle.background:			gray80
*Toggle.foreground:			gray20
*Toggle.borderWidth:			0
*Toggle.displayList:\
foreground	gray90;\
lines		1,-1,-1,-1,-1,1;\
foreground	gray40;\
lines		-1,0,0,0,0,-1

*mouse.bitmap:				mouse.xbm
*keyboard.bitmap:			keyboard.xbm
*card.bitmap:				card.xbm
*monitor.bitmap:			monitor.xbm

*Label.borderWidth:			0
*Viewport.forceBars:			True
*Viewport.allowVert:			True
*Viewport.useRight:			True

.xorgcfg.geometry:			320x400
.xorgcfg.minWidth:			320
.xorgcfg.minHeight:			400
.xorgcfg.maxWidth:			320
.xorgcfg.maxHeight:			400

.xorgcfg.config.geometry:		320x369
.xorgcfg.config.minWidth:		320
.xorgcfg.config.maxWidth:		320
.xorgcfg.config.minHeight:		369
.xorgcfg.config.maxHeight:		369

*work.width:				320
*work.height:				240

*error.label.label:\
Not all required fields\n\
were filled, or the specified\n\
identifier is duplicated.
*error.label.vertDistance:		30
*error.label.borderWidth:		0
*error.label.leftBitmap:		Excl
*error.command.fromVert:		label
*error.command.label:			Ok
*error.command.vertDistance:		20

*Scrollbar.translations:\
<BtnDown>:	StartScroll(Continuous) MoveThumb() NotifyThumb()\n\
<BtnMotion>:	MoveThumb() NotifyThumb()\n\
<BtnUp>:	NotifyScroll(Proportional) EndScroll()

*Scrollbar.background:		gray80
*Scrollbar.foreground:		rgb:a/5/5
*Scrollbar.borderWidth:		0
*Scrollbar.thumb:		vlines2
*Scrollbar.displayList:\
foreground	gray90;\
lines		1,-1,-1,-1,-1,1;\
foreground	gray40;\
lines		-1,0,0,0,0,-1

*Text.Translations: #override \
<Enter>:	no-op()\n\
<Leave>:	no-op()\n\
<Btn1Down>:	set-keyboard-focus() select-start()

*top.identifier.Translations: #override \
<Enter>:	no-op()\n\
<Leave>:	no-op()\n\
<Key>Return:	no-op()\n\
<Btn1Down>:	set-keyboard-focus() select-start()

*List.showCurrent:		True
*Tip.timeout:			100
*Tip.background:		rgb:f/f/8
*Tip.foreground:		gray20
*Tip.borderWidth:		0
*Tip.displayList:\
foreground	rgb:8/8/4;\
lines		1,-1,-1,-1,-1,1;\
foreground	rgb:f/f/c;\
lines		-1,0,0,0,0,-1

*Toggle.internalHeight:		2
*Toggle.internalWidth:		2
*mouse.label:			Mouse
*keyboard.label:		Keyboard
*card.label:			Card
*monitor.label:			Monitor

*commands.borderWidth:		0
*commands.defaultDistance:	2
*commands.?.bottom:		chainTop
*commands.height:		50

*commands.keyboard.fromHoriz:	mouse
*card.fromHoriz:		keyboard
*monitor.fromHoriz:		card

*commands.mouse*new.label:	Add new mouse
*commands.keyboard*new.label:	Add new keyboard
*commands.card*new.label:	Add new video card
*commands.monitor*new.label:	Add new monitor
*commands.mouse*configure.label:	Configure mouse(s)
*commands.keyboard*configure.label:	Configure keyboard(s)
*commands.card*configure.label:		Configure video card(s)
*commands.monitor*configure.label:	Configure monitor(s)
*commands.mouse*SimpleMenu*newMouse.label:	New mouse
*commands.keyboard*SimpleMenu*newKeyboard.label:New keyboard
*commands.card*SimpleMenu*newcard.label:	New card
*commands.monitor*SimpleMenu*newMonitor.label:	New monitor

*commands.MenuButton.translations: \
<Enter>:	highlight()\n\
<Leave>:	reset()\n\
Any<BtnDown>:	highlight() set() PopupMenu()

*hpane.showGrip:		False
*hpane.expert.label:		Expert Mode
*topM.min:			200
*topM.max:			200
*topM.justify:			left
*topM.label:			Configure Layout
*topM*layout.label:		Configure Layout
*topM*screen.label:		Configure Screen
*topM*modeline.label:		Configure Modeline
*topM*accessx.label:		Configure AccessX
*topM.showGrip:			False
*work.showGrip:			False

*MenuButton.leftBitmap:		menu10
*SmeBSB.HorizontalMargins:	18

*back.label:			<< Back
*next.label:			Next >>
*ok.label:			Ok
*cancel.label:			Cancel
*yes.label:			Yes
*no.label:			No

*help.label:			Help
*quit.label:			Quit
*next.fromHoriz:		back
*config*ok.fromHoriz:		next
*bottom*cancel.fromHoriz:	ok
*top.displayList:\
foreground	gray60;\
lines		1,-1,-1,-1,-1,1;\
foreground	white;\
lines		-1,0,0,0,0,-1
*bottom.displayList:\
foreground	gray60;\
lines		1,-1,-1,-1,-1,1;\
foreground	white;\
lines		-1,0,0,0,0,-1
*work.displayList:\
foreground	gray60;\
lines		1,-1,-1,-1,-1,1;\
foreground	white;\
lines		-1,0,0,0,0,-1
*options.pane.Form.displayList:\
foreground	gray60;\
lines		1,-1,-1,-1,-1,1;\
foreground	white;\
lines		-1,0,0,0,0,-1

*top.label.label:		Identifier:
*top.label.borderWidth:		0
*top.identifier.fromHoriz:	label
*top.label.internalHeight:	3
*top.label.justify:		left
*top.label.left:		chainLeft
*top.label.right:		chainLeft
*top.identifier.left:		chainLeft
*top.identifier.right:		chainRight

*bottom.layout.translations:	#override \
<Key>Return:	rename-layout()
*bottom.layout.label:		New server layout
*bottom.layout.tip:		Type a text and press Return to rename this layout
*bottom.Command.height:		19
*bottom.Text.height:		19
*bottom.MenuButton.height:	19
*bottom*new.label:		New server layout
*layout.fromHoriz:		select
*help.fromHoriz:		layout
*bottom.layout.justify:		left
*bottom.select.label:		Layout
*bottom.select.left:		chainLeft
*bottom.select.right:		chainLeft
*bottom.layout.left:		chainLeft
*quit.fromHoriz:		help
*bottom.?.left:			chainRight
*bottom.?.right:		chainRight
*bottom.?.top:			chainBottom
*bottom.?.bottom:		chainBottom

*pane.bottom.min:		30
*pane.bottom.max:		30
*pane.bottom.showGrip:		False
*pane.bottom.defaultDistance:	5

!! Wellcome message
*work.wellcome.borderWidth:	0
*work.wellcome.label:		Welcome to __VENDORNAME__ __VENDORVERS__ setup program
*work.?.borderWidth:		0
!*work.?.width:			310
*work.?.height:			290

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! mouseDP widget
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*mouseDP*Label.internalHeight:		0
*mouseDP*Label.borderWidth:		0
*mouseDP.labelD.vertDistance:		0
*mouseDP.labelD.label:			Select mouse device
*mouseDP.device.fromVert:		labelD
*mouseDP.viewportD.fromVert:		device
*mouseDP.device.width:			302
*mouseDP.viewportD.vertDistance:	2
*mouseDP.viewportD.listD.longest:	135
*mouseDP.viewportD.width:		302
*mouseDP.viewportD.height:		87
*mouseDP.labelP.label:			Select mouse protocol
*mouseDP.labelP.vertDistance:		10
*mouseDP.labelP.fromVert:		viewportD
*mouseDP.viewportP.fromVert:		labelP
*mouseDP.viewportP.forceBars:		True
*mouseDP.viewportP.allowVert:		True
*mouseDP.viewportP.useRight:		True
*mouseDP.viewportP.listP.longest:	135
*mouseDP.viewportP.width:		302
*mouseDP.viewportP.height:		110
*mouseDP.viewportP.vertDistance:	6
*mouseDP*List.verticalList:		True
*mouseDP.emulate3.fromVert:		viewportP
*mouseDP.emulate3.width:		180
*mouseDP.apply.label:			Apply changes
*mouseDP.apply.fromVert:		viewportP
*mouseDP.apply.fromHoriz:		emulate3
*mouseDP.apply.width:			116
*mouseDP.emulate3.vertDistance:		10
*mouseDP.apply.vertDistance:		10
*mouseDP.emulate3.label:		Emulate 3 buttons
*mouseDP.emulate3.tip:			Select if your mouse has only two buttons

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! keyboardML widget
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*keyboardML.Label.vertDistance:		8
*keyboardML.MenuButton.vertDistance:	8
*keyboardML.keyboard.vertDistance:	10
*keyboardML.Label.borderWidth:		0
*keyboardML.Label.width:		115
*keyboardML.MenuButton.width:		185
*keyboardML.MenuButton.justify:		left
*keyboardML.Label.internalWidth:	0
*keyboardML.Label.justify:		right
*keyboardML.labelR.label:		Xkb rules:
*keyboardML.rules.left:			chainRight
*keyboardML.rules.right:		chainRight
*keyboardML.rules.fromHoriz:		labelR
*keyboardML.model.fromVert:		rules
*keyboardML.labelM.fromVert:		rules
*keyboardML.model.left:			chainRight
*keyboardML.model.right:		chainRight
*keyboardML.model.fromHoriz:		labelM
*keyboardML.labelM.label:		Keyboard model:
*keyboardML.layout.left:		chainRight
*keyboardML.layout.right:		chainRight
*keyboardML.layout.fromVert:		model
*keyboardML.labelL.fromVert:		model
*keyboardML.layout.fromHoriz:		labelL
*keyboardML.labelL.label:		Keyboard layout:
*keyboardML.variant.fromVert:		labelL
*keyboardML.variant.fromHoriz:		labelV
*keyboardML.labelV.fromVert:		labelL
*keyboardML.labelV.label:		Xkb variant:
*keyboardML.options.fromVert:		labelV
*keyboardML.options.fromHoriz:		labelO
*keyboardML.labelO.fromVert:		labelV
*keyboardML.labelO.label:		Xkb options:
*keyboardML.keyboard.fromVert:		labelO
*keyboardML.keyboard.borderWidth:	0
*keyboardML.keyboard.width:		305
*keyboardML.keyboard.height:		121
*keyboardML.apply.vertDistance:		16
*keyboardML.apply.fromVert:		keyboard
*keyboardML.apply.label:		Apply changes

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! cardModel widget
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*cardModel.label.internalHeight:	0
*cardModel.keyboard.borderWidth:	0
*cardModel.label.label:			Card model filter
*cardModel.label.internalHeight:	2
*cardModel.label.borderWidth:		0
*cardModel.viewport.vertDistance:	2
*cardModel.viewport.fromVert:		filter
*cardModel.filter.fromHoriz:		label
*cardModel.filter.width:		171
*cardModel.viewport.width:		302
*cardModel.viewport.height:		212
*cardModel.driver.justify:		left
*cardModel.driverL.label:		Driver
*cardModel.driverL.fromVert:		viewport
*cardModel.driverL.width:		50
*cardModel.driverL.justify:		right
*cardModel.driver.fromVert:		viewport
*cardModel.driver.fromHoriz:		driverL
*cardModel.driver.width:		250
*cardModel.driver.left:			chainRight
*cardModel.driver.right:		chainRight
*cardModel.busidL.label:		BusID
*cardModel.busidL.fromVert:		driver
*cardModel.busidL.width:		50
*cardModel.busidL.justify:		right
*cardModel.busid.fromVert:		driver
*cardModel.busid.fromHoriz:		busidL
*cardModel.busid.width:			250
*cardModel.busid.left:			chainRight
*cardModel.busid.right:			chainRight
*cardModel.viewport.forceBars:		True
*cardModel.viewport.allowVert:		True
*cardModel.viewport.useRight:		True
*cardModel.viewport.list.longest:	277
*cardModel.filter.tip:			Type name or vendor of your card and press enter
*cardModel.filter.translations:	#override \
<Key>Return:	filter-card()


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! main widget
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*work.cpu.backgroundPixmap:		computer.xpm
*work.mouse.backgroundPixmap:		mouse.xpm
*work.keyboard.backgroundPixmap:	keyboard.xpm
*work.card.backgroundPixmap:		card.xpm
*work.monitor.backgroundPixmap:		monitor.xpm

*work.cpu.x:				130
*work.cpu.y:				160
*work.cpu.width:			30
*work.cpu.height:			50
*work.mouse.width:			26
*work.mouse.height:			35
*work.keyboard.width:			60
*work.keyboard.height:			28
*work.card.width:			41
*work.card.height:			40
*work.monitor.width:			47
*work.monitor.height:			40

.xorgcfg.pane.work.Simple.borderWidth:	1
.xorgcfg.pane.work.Simple.translations:\
Any<Btn1Down>:	select-device()\n\
Any<Btn1Motion>:	move-device()\n\
Any<Btn1Up>:	unselect-device()\n\
Any<Btn3Down>:	device-popup()\n\
Any<Btn3Up>:	device-popdown()

.xorgcfg.pane.work.screen.translations:\
Any<Btn1Down>:	select-device()\n\
Any<Btn1Motion>:	move-device()\n\
Any<Btn1Up>:	unselect-device()\n\
Any<Btn3Down>:	device-popup()\n\
Any<Btn3Up>:	device-popdown()

Xorgcfg.translations:	#override \
<Message>WM_PROTOCOLS:	quit()
.xorgcfg.config.translations:	#override \
<Message>WM_PROTOCOLS:	config-cancel()
.xorgcfg.options.translations:	#override \
<Message>WM_PROTOCOLS:	options-cancel()
.xorgcfg.quit.translations:	#override \
<Message>WM_PROTOCOLS:	quit-cancel()
.xorgcfg.error.translations:	#override \
<Message>WM_PROTOCOLS:	error-cancel()
.xorgcfg.force.translations:	#override \
<Message>WM_PROTOCOLS:	addmode-cancel()
.xorgcfg.addMode.translations:	#override \
<Message>WM_PROTOCOLS:	addmode-cancel()
.xorgcfg.accessx.translations:	#override \
<Message>WM_PROTOCOLS:	accessx-close()
.xorgcfg.test.translations:	#override \
<Message>WM_PROTOCOLS:	testmode-cancel()
.xorgcfg.Expert.translations:	#override \
<Message>WM_PROTOCOLS:	expert-close()
.xorgcfg.options.moduleOptions.translations:	#override \
<Message>WM_PROTOCOLS:	module-options-close()


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Options
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
.xorgcfg.options.geometry:		400x176
.xorgcfg.options.minWidth:		400
.xorgcfg.options.maxWidth:		400
.xorgcfg.options.minHeight:		176
.xorgcfg.options.maxHeight:		176

*options*commands.remove.fromHoriz:	add
*options*commands.update.fromHoriz:	remove
*options*commands.help.fromHoriz:	update
*options*commands.min:			30
*options*commands.max:			30
*options*commands.showGrip:		False
*options*commands.defaultDistance:	4
*options*commands.?.width:		100
*options*commands.?.height:		20

*options*name.fromHoriz:		label1
*options*label2.fromHoriz:		name
*options*value.fromHoriz:		label2
*options*label3.fromHoriz:		value
*options*viewport.fromVert:		name
*options*Label.borderWidth:		0
*options.pane.form.Label.internalWidth:	0
*options.pane.form.Label.height:	21
*options.pane.form.Text.height:		19
*options.pane.form.Label.top:		chainTop
*options.pane.form.Text.top:		chainTop
*options.pane.form.Label.bottom:	chainTop
*options.pane.form.Text.bottom:		chainTop
*options.pane.form.Label.horizDistance:	0
*options.pane.form.Text.horizDistance:	0
*options.pane.form.Label.vertDistance:	8
*options.pane.form.Text.vertDistance:	8
*options.pane.form.Text.width:		147
*options*viewport.width:		390
*options*viewport.height:		50

*options*viewport.horizDistance:	7
*options*label1.horizDistance:		5
*options*viewport.left:			chainLeft
*options*viewport.right:		chainRight
*options*list.longest:			376

*options*driverOpts.label:		Options for module
*options*driverOpts.justify:		left
*options*driverOpts.width:		278
*options*popdown.label:			Popdown dialog
*options*driverOpts.tip:		This menu shows:\n\
 o option name\n\
 o option type

.xorgcfg*options.moduleOptions.geometry:			360x245
.xorgcfg*options.moduleOptions.minWidth:			360
.xorgcfg*options.moduleOptions.maxWidth:			360
.xorgcfg*options.moduleOptions.minHeight:			245
.xorgcfg*options.moduleOptions.maxHeight:			245
*options.moduleOptions*descriptions*labelType.label:		Select option
*options.moduleOptions*descriptions*labelType.width:		348
*options.moduleOptions*descriptions*module.fromVert:		labelType
*options.moduleOptions*descriptions*module.label:		Module
*options.moduleOptions*descriptions*Label.horizDistance:	8
*options.moduleOptions*descriptions*option.fromVert:		labelType
*options.moduleOptions*descriptions*option.label:		Option
*options.moduleOptions*descriptions*option.fromHoriz:		viewM
*options.moduleOptions*descriptions*viewM.fromVert:		module
*options.moduleOptions*descriptions*viewM.width:		120
*options.moduleOptions*descriptions*viewM.height:		94
*options.moduleOptions*descriptions*modL.longest:		100
*options.moduleOptions*descriptions*viewO.fromHoriz:		viewM
*options.moduleOptions*descriptions*viewO.fromVert:		option
*options.moduleOptions*descriptions*viewO.width:		220
*options.moduleOptions*descriptions*viewO.height:		94
*options.moduleOptions*descriptions*optL.longest:		220
*options.moduleOptions*descriptions*desc.horizDistance:		4
*options.moduleOptions*descriptions*desc.fromVert:		viewM
*options.moduleOptions*descriptions*desc.width:			348
*options.moduleOptions*descriptions*desc.height:		62
*options.moduleOptions*descriptions*desc.wrap:			word
*options.moduleOptions*descriptions*desc.scrollVertical:	Always
*options.moduleOptions*descriptions*desc.Scrollbar.foreground:	rgb:a/5/5
*options.moduleOptions*descriptions*desc.Scrollbar.borderWidth:	1
*options.moduleOptions*descriptions*desc.Scrollbar.borderColor:	gray60
*options.moduleOptions*descriptions.showGrip:			False
*options.moduleOptions*popdown.label:				Popdown dialog

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! monitor
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*monitorl*Viewport.width:		302
*monitorl*Text.width:			160
*monitorl*Text.tip:\
Select standard value from the list\n\
below, or enter specific value here.

*monitorl*hlabel.justify:		right
*monitorl*hlabel.width:			138
*monitorl*hlabel.label:			Horizontal sync
*monitorl*hsync.fromHoriz:		hlabel
*monitorl*hviewport.fromVert:		hsync
*monitorl*hviewport.height:		156
*monitorl*hlist.longest:		288

*monitorl*vlabel.justify:		right
*monitorl*vlabel.width:			138
*monitorl*vlabel.vertDistance:		12
*monitorl*vsync.vertDistance:		12
*monitorl*vlabel.fromVert:		hviewport
*monitorl*vsync.fromVert:		hviewport
*monitorl*vlabel.label:			Vertical sync
*monitorl*vsync.fromHoriz:		vlabel
*monitorl*vviewport.fromVert:		vsync
*monitorl*vviewport.height:		20
*monitorl*vlist.longest:		64

*monitorl*clabel.vertDistance:		12
*monitorl*clabel.fromVert:		vviewport
*monitorl*clabel.label:			Select card connected to monitor
*monitorl*cmenu.fromVert:		clabel
*monitorl*cmenu.width:			302
*monitorl*cmenu.justify:		left
*monitorl*none.label:			None

!! vidtune
*vidtune.Repeater.borderWidth:		1
*vidtune.Repeater.borderColor:		gray90
*vidtune.Repeater.shapeStyle:		ellipse
*vidtune.Repeater.foreground:		gray30
*vidtune.Repeater.background:		gray80
*vidtune.Repeater.width:		31
*vidtune.Repeater.height:		29
*vidtune.Repeater.repeatDelay:		200
*vidtune.Repeater.decay:		0
*vidtune.Repeater.displayList:\
line-width	2;\
foreground	gray65;\
draw-arc	0,0,-0,-0,230,180;\
foreground	gray95;\
draw-arc	0,0,-0,-0,40,180;\
foreground	gray80;\
draw-arc	0,0,-0,-0,220,10;\
draw-arc	0,0,-0,-0,40,10
*vidtune.Repeater.translations:\
<Enter>:	set-values(1, borderColor, gray50)\n\
<Leave>:	set-values(1, borderColor, gray90)\n\
<Btn1Down>:	set-values(1, borderColor, gray90) set() start()\n\
<Btn1Up>:	stop() unset()
*vidtune.left.bitmap:			left.xbm
*vidtune.right.bitmap:			right.xbm
*vidtune.up.bitmap:			up.xbm
*vidtune.down.bitmap:			down.xbm
*vidtune.wider.bitmap:			wider.xbm
*vidtune.narrower.bitmap:		narrower.xbm
*vidtune.shorter.bitmap:		shorter.xbm
*vidtune.taller.bitmap:			taller.xbm

*vidtune.vesaB.label:			Add standard VESA mode to current screen
*vidtune.vesaB.width:			312

*vidtune.screenB.fromVert:		mode
*vidtune.screenB.width:			160
*vidtune.screenB.horizDistance:		80
*vidtune.prev.fromVert:			vesaB
*vidtune.mode.fromVert:			vesaB
*vidtune.next.fromVert:			vesaB
*vidtune.prev.horizDistance:		54
*vidtune.prev.label:			<<
*vidtune.mode.fromHoriz:		prev
*vidtune.mode.width:			160
*vidtune.next.label:			>>
*vidtune.next.fromHoriz:		mode

*vidtune.up.fromVert:			screenB
*vidtune.up.horizDistance:		143
*vidtune.left.horizDistance:		98
*vidtune.left.vertDistance:		16
*vidtune.left.fromVert:			up
*vidtune.monitor.fromVert:		up
*vidtune.monitor.vertDistance:		0
*vidtune.monitor.fromHoriz:		left
*vidtune.right.vertDistance:		16
*vidtune.right.fromVert:		up
*vidtune.right.fromHoriz:		monitor
*vidtune.down.horizDistance:		143
*vidtune.down.fromVert:			monitor
*vidtune.wider.fromVert:		left
*vidtune.wider.horizDistance:		91
*vidtune.narrower.fromVert:		down
*vidtune.narrower.fromHoriz:		wider
*vidtune.shorter.fromVert:		down
*vidtune.shorter.fromHoriz:		narrower
*vidtune.taller.fromVert:		right
*vidtune.taller.fromHoriz:		shorter
*vidtune.monitor.width:			47
*vidtune.monitor.height:		40
*vidtune.monitor.backgroundPixmap:	monitor.xpm
*vidtune.narrower.horizDistance:	0
*vidtune.shorter.horizDistance:		2
*vidtune.taller.horizDistance:		0
*vidtune.wider.vertDistance:		20
*vidtune.taller.vertDistance:		20
*vidtune.narrower.vertDistance:		0
*vidtune.shorter.vertDistance:		0
*vidtune.down.vertDistance:		0

*vidtune.monitor.tip:\
\                  WARNING\n\
\  Using the  controls here  may damage your\n\
monitor.  You can safely skip  this section\n\
of the configuration process.\n\
\n\
Press ESC if your monitor goes out of sync.

*vidtune.background:			white
*vidtune.form.borderWidth:		0
*vidtune.form.background:		white
*vidtune.form.defaultDistance:		0
*vidtune.form.vertDistance:		3
*vidtune.form.horizDistance:		4
*vidtune.form.Label.foreground:		gray20
*vidtune.form.Label.background:		white
*vidtune.form.Label.font:		-*-fixed-*-*-*-*-10-*-*-*-*-*-*-1
*vidtune.form.Label.vertDistance:	0
*vidtune*Label.justify:			right

*vidtune.form.fromVert:			auto
*vidtune*hsyncstart.label:		HSyncStart:
*vidtune*hsyncstart.width:		95
*vidtune*v-hsyncstart.width:		40
*vidtune*v-hsyncstart.fromHoriz:	hsyncstart
*vidtune*vsyncstart.fromHoriz:		v-hsyncstart
*vidtune*v-vsyncstart.fromHoriz:	vsyncstart
*vidtune*vsyncstart.label:		VSyncStart:
*vidtune*vsyncstart.width:		95
*vidtune*v-vsyncstart.width:		40
*vidtune*hsyncend.label:		HSyncEnd:
*vidtune*hsyncend.width:		95
*vidtune*v-hsyncend.width:		40
*vidtune*v-hsyncend.fromHoriz:		hsyncend
*vidtune*hsyncend.fromVert:		hsyncstart
*vidtune*v-hsyncend.fromVert:		v-hsyncstart
*vidtune*vsyncend.label:		VSyncEnd:
*vidtune*vsyncend.width:		95
*vidtune*v-vsyncend.width:		40
*vidtune*vsyncend.fromHoriz:		v-hsyncend
*vidtune*v-vsyncend.fromHoriz:		vsyncend
*vidtune*vsyncend.fromVert:		hsyncstart
*vidtune*v-vsyncend.fromVert:		v-vsyncstart
*vidtune*htotal.label:			HTotal:
*vidtune*htotal.width:			95
*vidtune*v-htotal.width:		40
*vidtune*v-htotal.fromHoriz:		htotal
*vidtune*htotal.fromVert:		hsyncend
*vidtune*v-htotal.fromVert:		v-hsyncend
*vidtune*vtotal.label:			VTotal:
*vidtune*vtotal.width:			95
*vidtune*v-vtotal.width:		40
*vidtune*vtotal.fromHoriz:		v-htotal
*vidtune*v-vtotal.fromHoriz:		vtotal
*vidtune*vtotal.fromVert:		vsyncend
*vidtune*v-vtotal.fromVert:		v-vsyncend
*vidtune*flags.label:			Flags:
*vidtune*flags.width:			142
*vidtune*v-flags.width:			156
*vidtune*v-flags.justify:		left
*vidtune*v-flags.fromHoriz:		flags
*vidtune*flags.fromVert:		vtotal
*vidtune*v-flags.fromVert:		v-vtotal
*vidtune*clock.label:			Pixel Clock (MHz):
*vidtune*clock.width:			142
*vidtune*v-clock.width:			48
*vidtune*v-clock.fromHoriz:		clock
*vidtune*clock.fromVert:		flags
*vidtune*v-clock.fromVert:		v-flags
*vidtune*hsync.label:			Horizontal Sync (kHz):
*vidtune*hsync.width:			142
*vidtune*v-hsync.width:			48
*vidtune*v-hsync.fromHoriz:		hsync
*vidtune*hsync.fromVert:		clock
*vidtune*v-hsync.fromVert:		v-clock
*vidtune*vsync.label:			Vertical Sync (Hz):
*vidtune*vsync.width:			142
*vidtune*v-vsync.width:			48
*vidtune*v-vsync.fromHoriz:		vsync
*vidtune*vsync.fromVert:		hsync
*vidtune*v-vsync.fromVert:		v-hsync

*vidtune.auto.horizDistance:		43
*vidtune.auto.fromVert:			narrower
*vidtune.auto.label:			Auto
!*vidtune.auto.state:			True
*vidtune.apply.fromVert:		narrower
*vidtune.apply.fromHoriz:		auto
*vidtune.apply.label:			Apply
*vidtune.restore.fromHoriz:		apply
*vidtune.restore.horizDistance:		4
*vidtune.restore.fromVert:		narrower
*vidtune.restore.label:			Restore
*vidtune.update.fromVert:		narrower
*vidtune.update.fromHoriz:		restore
*vidtune.update.label:			Update
*vidtune.test.fromVert:			narrower
*vidtune.test.fromHoriz:		update
*vidtune.test.label:			Test

*vidtune.Label.background:		white
*vidtune.addto.vertDistance:		4
*vidtune.addto.fromHoriz:		add
*vidtune.addto.fromVert:		form
*vidtune.addto.label:			mode to
*vidtune.addto.horizDistance:		0
*vidtune.ident.vertDistance:		4
*vidtune.ident.horizDistance:		0
*vidtune.ident.fromVert:		form
*vidtune.ident.fromHoriz:		addto
*vidtune.ident.width:			98
*vidtune.ident.justify:			left
*vidtune.as.vertDistance:		4
*vidtune.as.horizDistance:		0
*vidtune.as.fromVert:			form
*vidtune.as.fromHoriz:			ident
*vidtune.as.label:			as:
*vidtune.text.vertDistance:		4
*vidtune.text.horizDistance:		0
*vidtune.text.fromVert:			form
*vidtune.text.fromHoriz:		as
*vidtune.text.width:			98
*vidtune.add.vertDistance:		4
*vidtune.add.fromVert:			form
*vidtune.add.label:			Add
.xorgcfg.force.geometry:		268x58
.xorgcfg.force.minWidth:		268
.xorgcfg.force.maxWidth:		268
.xorgcfg.force.minHeight:		58
.xorgcfg.force.maxHeight:		58
.xorgcfg.force.?.label:			There is already a modeline with the\n\
specified identifier. Add anyway?

.xorgcfg.addMode.geometry:		350x80
.xorgcfg.addMode.minWidth:		350
.xorgcfg.addMode.maxWidth:		350
.xorgcfg.addMode.minHeight:		80
.xorgcfg.addMode.maxHeight:		80
.xorgcfg.addMode.?.label:		XF86VidModeAddModeLine returned True,\n\
but no modeline was added to the current Screen.\n\
Do you want to add it to the Monitor section?

.xorgcfg.test.?.label:			\        Testing modeline...\n\n\
Press ESC or stop button to quit.
.xorgcfg.test.geometry:			250x72
.xorgcfg.test.minWidth:			250
.xorgcfg.test.maxWidth:			250
.xorgcfg.test.minHeight:		72
.xorgcfg.test.maxHeight:		72

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! screen
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*screenD*depthL.label:			Default color depth:
*screenD*depthL.height:			20
*screenD*1.fromHoriz:			depthL
*screenD*4.fromHoriz:			1
*screenD*8.fromHoriz:			4
*screenD*16.fromHoriz:			8
*screenD*24.fromHoriz:			16
*screenD*1.width:			24
*screenD*4.width:			24
*screenD*8.width:			24
*screenD*16.width:			24
*screenD*24.width:			24
*screenD*1.horizDistance:		5
*screenD*4.horizDistance:		5
*screenD*8.horizDistance:		5
*screenD*16.horizDistance:		5
*screenD*24.horizDistance:		5
*screenD*modeL.fromVert:		depthL
*screenD*modeL.vertDistance:		14
*screenD*modeL.label:			Select resolution(s):
*screenD*viewL.fromVert:		modeL
*screenD*select.fromHoriz:		viewL
*screenD*unselect.fromHoriz:		viewL
*screenD*select.fromVert:		modeL
*screenD*unselect.fromVert:		select
*screenD*up.fromHoriz:			viewL
*screenD*down.fromHoriz:		viewL
*screenD*up.fromVert:			unselect
*screenD*down.fromVert:			up
*screenD*viewR.fromHoriz:		select
*screenD*viewR.fromVert:		modeL
*screenD*select.bitmap:			right.xbm
*screenD*unselect.bitmap:		left.xbm
*screenD*up.bitmap:			up.xbm
*screenD*down.bitmap:			down.xbm
*screenD*viewL.width:			133
*screenD*viewR.width:			133
*screenD*viewL.height:			184
*screenD*viewR.height:			184
*screenD*listLeft.longest:		128
*screenD*listRight.longest:		128
*screenD*rotate.tip:			Don't select any option if\n\
your monitor is not rotated.
*screenD*rotate.vertDistance:		14
*screenD*rotate.fromVert:		viewL
*screenD*rotate.label:			Rotate screen:
*screenD*CW.tip:			Clock wise
*screenD*CW.vertDistance:		14
*screenD*CW.fromVert:			viewL
*screenD*CW.fromHoriz:			rotate
*screenD*CW.label:			CW
*screenD*CW.width:			40
*screenD*CCW.tip:			Counter-clock wise
*screenD*CCW.vertDistance:		14
*screenD*CCW.fromVert:			viewL
*screenD*CCW.fromHoriz:			CW
*screenD*CCW.label:			CCW
*screenD*CCW.width:			40

*work.screen.width:			100
*work.screen.height:			80


*Dialog.background:			gray85
*quit.ask.label:			Write configuration to
*quit.ask.value.translations:	#override \
<Key>Return:	write-config()
*quit.ask.label.justify:		left
*quit.ask.value.width:			222
*quit.ask.icon:				Excl
.xorgcfg.quit.geometry:			230x92
.xorgcfg.quit.minWidth:			230
.xorgcfg.quit.maxWidth:			230
.xorgcfg.quit.minHeight:		92
.xorgcfg.quit.maxHeight:		92

*error.notice.label:			Failed to write configuration file.
.xorgcfg.error.geometry:		280x50
.xorgcfg.error.minWidth:		260
.xorgcfg.error.maxWidth:		260
.xorgcfg.error.minHeight:		50
.xorgcfg.error.maxHeight:		50

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! accessx
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*Accessx*Label.font:			-*-fixed-medium-r-*-*-13-*-*-*-*-*-iso8859-1
*Accessx.Form.Toggle.font:		-*-fixed-medium-r-*-*-13-*-*-*-*-*-iso8859-1
*Accessx*Label.background:		white
*Accessx*Label.justify:			left
*Accessx*Label.internalHeight:		0
*Accessx*Label.internalWidth:		0
*Accessx*Label.foreground:		gray35
*Accessx*Toggle.internalHeight:		0
*Accessx.Toggle.internalWidth:		18
*Accessx.Toggle.justify:		left
*Accessx.Toggle.foreground:		gray30
*Accessx.Toggle.background:		white
*Accessx*Toggle.highlightThickness:	0
*Accessx.Toggle.foreground:		rgb:f/7/7
*Accessx.Toggle.displayList:\
foreground	white;\
points		0,0,-1,0;\
foreground	rgb:f/7/7;\
fill-poly	0,2,0,-3,8,6
*Accessx.Toggle.translations: \
<Btn1Down>,<Btn1Up>: toggle() notify()\n\
<Enter>: set-values(1, displayList, "foreground white;points 0,0,-1,0;fill-poly 0,1,10,6,0,-2;foreground rgb:7/7/f;fill-poly 0,2,0,-3,8,6")\n\
<Leave>: set-values(1, displayList, "foreground white;points 0,0,-1,0;foreground rgb:f/7/7;fill-poly 0,2,0,-3,8,6")
*Accessx.Form.borderWidth:		1
*Accessx.Form.borderColor:		rgb:f/a/a
*Accessx.Form.vertDistance:		0
*Accessx.Form.defaultDistance:		2
*Accessx.borderWidth:			0
*accessxForm.background:		white
*Accessx*Form.background:		white
*Accessx.background:			white
*accessxForm.enable.label:		Enable AccessX
*accessxForm.enable.width:		208
*accessxForm.apply.fromHoriz:		enable
*accessxForm.apply.label:		Apply changes
*accessxForm.Accessx.fromVert:		enable
*Accessx.timeoutToggle.background:	white
*Accessx.timeoutToggle.foreground:	rgb:7/7/f
*Accessx.timeoutToggle.displayList:\
foreground	rgb:a/a/f;\
lines		1,-1,-1,-1,-1,1;\
foreground	rgb:2/2/a;\
lines		-1,0,0,0,0,-1

*Accessx.Form.Toggle.background:	white
*Accessx.Form.Toggle.foreground:	rgb:7/7/f
*Accessx.Form.Toggle.displayList:\
foreground	rgb:a/a/f;\
lines		1,-1,-1,-1,-1,1;\
foreground	rgb:2/2/a;\
lines		-1,0,0,0,0,-1
*Accessx.Form.Toggle.translations: \
<Btn1Down>,<Btn1Up>: toggle() notify()\n\
<Enter>: set-values(1, displayList, "foreground rgb:a/a/f;lines 1,-1,-1,-1,-1,1;foreground rgb:2/2/a;lines -1,0,0,0,0,-1;foreground rgb:f/7/7;draw-rect 1,1,-2,-2")\n\
<Leave>: set-values(1, displayList, "foreground rgb:a/a/f;lines 1,-1,-1,-1,-1,1;foreground rgb:2/2/a;lines -1,0,0,0,0,-1")

*Accessx.timeoutToggle.internalWidth:	4
*Accessx.timeoutToggle.translations:\
<Btn1Down>,<Btn1Up>:	toggle() notify()
*Accessx.timeoutToggle.internalWidth:	4
*Accessx.timeoutToggle.internalHeight:	1
*Accessx.timeoutToggle.vertDistance:	6
*Accessx.timeoutToggle.label:		Time Out
*Accessx.timeoutLabel.fromHoriz:	timeoutToggle
*Accessx.timeoutLabel.vertDistance:	7
*Accessx.timeoutLabel.label:		Time (min)
*Accessx.Label.background:		white
*Accessx.timeoutNumber.vertDistance:	1
*Accessx.timeoutNumber.fromHoriz:	timeoutLabel
*Accessx.timeoutNumber.font:		6x9
*Accessx.timeoutNumber.label:		??
*Accessx.timeoutScroller.fromVert:	timeoutNumber
*Accessx.timeoutScroller.fromHoriz:	timeoutLabel
*Accessx.timeoutScroller.width:		133
*Accessx.timeoutScroller.horizDistance:	31
*Accessx*Scrollbar.vertDistance:	0
*Accessx*Scrollbar.orientation:		horizontal
*Accessx*Scrollbar.thumb:		black
*Accessx*Scrollbar.height:		8
*Accessx*Scrollbar.minimumThumb:	5
*Accessx*Scrollbar.borderWidth:		1
*Accessx*Scrollbar.borderColor:		white
*Accessx*Scrollbar.foreground:		rgb:f/7/7
*Accessx*Scrollbar.background:		gray95
*Accessx*Scrollbar.displayList:\
foreground	gray80;\
lines		1,-1,-1,-1,-1,1;\
foreground	gray90;\
lines		-1,0,0,0,0,-1
*Accessx.sticky.fromVert:		timeoutToggle
*Accessx.sticky.vertDistance:		6
*Accessx.sticky.label:			Enable StickyKeys
*Accessx.sticky.width:			304
*Accessx.stickyForm.fromVert:		sticky
*Accessx.stickyForm.Toggle.width:	148
*Accessx.stickyForm.Toggle.internalHeight:	1
*Accessx.stickyForm.auto.label:		Auto off
*Accessx.stickyForm.beep.fromHoriz:	auto
*Accessx.stickyForm.beep.label:		Modifiers beep
*Accessx.mouseKeys.fromVert:		stickyForm
*Accessx.mouseKeys.width:		304
*Accessx.mouseKeys.vertDistance:	6
*Accessx.mouseKeys.label:		Enable MouseKeys
*Accessx.mouseForm.fromVert:		mouseKeys
*Accessx.mouseForm.speedLabel.vertDistance:	7
*Accessx.mouseForm.speedLabel.label:	Peak speed (pixels/sec)
*Accessx.mouseForm.speedNumber.fromHoriz:	speedLabel
*Accessx.mouseForm.speedNumber.font:	6x9
*Accessx.mouseForm.speedNumber.label:		???
*Accessx.mouseForm.speedScroller.fromHoriz:	speedLabel
*Accessx.mouseForm.speedScroller.fromVert:	speedNumber
*Accessx.mouseForm.speedScroller.width:		133
*Accessx.mouseForm.timeLabel.fromVert:	speedScroller
*Accessx.mouseForm.timeLabel.label:	Time to peak (sec)
*Accessx.mouseForm.timeLabel.vertDistance:	7
*Accessx.mouseForm.timeNumber.label:	???
*Accessx.mouseForm.timeNumber.fromVert:	speedScroller
*Accessx.mouseForm.timeNumber.fromHoriz:	timeLabel
*Accessx.mouseForm.timeNumber.font:	6x9
*Accessx.mouseForm.timeScroller.fromHoriz:	timeLabel
*Accessx.mouseForm.timeScroller.fromVert:	timeNumber
*Accessx.mouseForm.timeScroller.width:	133
*Accessx.mouseForm.timeScroller.horizDistance:	37
*Accessx.mouseForm.delayLabel.fromVert:	timeScroller
*Accessx.mouseForm.delayLabel.label:	Motion delay (sec)
*Accessx.mouseForm.delayLabel.tip:\
Time between the initial key press\n\
and the first repeated motion event
*Accessx.mouseForm.delayLabel.vertDistance:	7
*Accessx.mouseForm.delayNumber.label:	???
*Accessx.mouseForm.delayNumber.fromVert:	timeScroller
*Accessx.mouseForm.delayNumber.fromHoriz:	delayLabel
*Accessx.mouseForm.delayNumber.font:	6x9
*Accessx.mouseForm.delayScroller.fromHoriz:	delayLabel
*Accessx.mouseForm.delayScroller.fromVert:	delayNumber
*Accessx.mouseForm.delayScroller.width:	133
*Accessx.mouseForm.delayScroller.horizDistance:	37
*Accessx.repeatKeys.fromVert:		mouseForm
*Accessx.repeatKeys.width:		304
*Accessx.repeatKeys.vertDistance:	6
*Accessx.repeatKeys.label:		Enable RepeatKeys
*Accessx.repeatForm.fromVert:		repeatKeys
*Accessx.repeatForm.rateLabel.vertDistance:	7
*Accessx.repeatForm.rateLabel.label:	Repeat rate (sec/key)
*Accessx.repeatForm.rateNumber.fromHoriz:	rateLabel
*Accessx.repeatForm.rateNumber.font:	6x9
*Accessx.repeatForm.rateNumber.label:	????
*Accessx.repeatForm.rateScroller.fromHoriz:	rateLabel
*Accessx.repeatForm.rateScroller.fromVert:	rateNumber
*Accessx.repeatForm.rateScroller.width:	133
*Accessx.repeatForm.rateScroller.horizDistance:	16
*Accessx.repeatForm.delayLabel.fromVert:	rateScroller
*Accessx.repeatForm.delayLabel.label:	Repeat delay (sec)
*Accessx.repeatForm.delayLabel.vertDistance:	7
*Accessx.repeatForm.delayNumber.label:	????
*Accessx.repeatForm.delayNumber.fromVert:	rateScroller
*Accessx.repeatForm.delayNumber.fromHoriz:	delayLabel
*Accessx.repeatForm.delayNumber.font:	6x9
*Accessx.repeatForm.delayScroller.fromHoriz:	delayLabel
*Accessx.repeatForm.delayScroller.fromVert:	delayNumber
*Accessx.repeatForm.delayScroller.width:	133
*Accessx.repeatForm.delayScroller.horizDistance:	37
*Accessx.slow.fromVert:			repeatForm
*Accessx.slow.vertDistance:		6
*Accessx.slow.label:			Enable SlowKeys
*Accessx.slow.width:			304
*Accessx.slowForm.fromVert:		slow
*Accessx.slowForm.Toggle.horizDistance:	4
*Accessx.slowForm.Toggle.internalWidth:	4
*Accessx.slowForm.Toggle.internalHeight:	1
*Accessx.slowForm.beep.label:		Beep when key is
*Accessx.slowForm.beep.vertDistance:	3
*Accessx.slowForm.pressed.fromHoriz:	beep
*Accessx.slowForm.pressed.label:	pressed
*Accessx.slowForm.accepted.fromHoriz:	pressed
*Accessx.slowForm.accepted.label:	accepted
*Accessx.slowForm.slowLabel.fromVert:	accepted
*Accessx.slowForm.slowLabel.label:	Key delay (sec)
*Accessx.slowForm.slowLabel.vertDistance:	7
*Accessx.slowForm.slowNumber.label:	???
*Accessx.slowForm.slowNumber.fromVert:	accepted
*Accessx.slowForm.slowNumber.fromHoriz:	slowLabel
*Accessx.slowForm.slowNumber.font:	6x9
*Accessx.slowForm.slowScroller.fromHoriz:	slowLabel
*Accessx.slowForm.slowScroller.fromVert:	slowNumber
*Accessx.slowForm.slowScroller.width:	133
*Accessx.slowForm.slowScroller.horizDistance:	58
*Accessx.bounce.fromVert:		slowForm
*Accessx.bounce.vertDistance:		6
*Accessx.bounce.label:			Enable BounceKeys
*Accessx.bounce.width:			304
*Accessx.bounceForm.fromVert:		bounce
*Accessx.bounceForm.bounceLabel.label:	Debounce time (sec)
*Accessx.bounceForm.bounceLabel.vertDistance:	7
*Accessx.bounceForm.bounceNumber.label:	???
*Accessx.bounceForm.bounceNumber.fromHoriz:	bounceLabel
*Accessx.bounceForm.bounceNumber.font:	6x9
*Accessx.bounceForm.bounceScroller.fromHoriz:	bounceLabel
*Accessx.bounceForm.bounceScroller.fromVert:	bounceNumber
*Accessx.bounceForm.bounceScroller.width:	133
*Accessx.bounceForm.bounceScroller.horizDistance:	30

*accessx.geometry:			220x253
*accessx.minWidth:			220
*accessx.maxWidth:			220
*accessx.minHeight:			253
*accessx.maxHeight:			253
*accessx*lock.fromVert:			label
*accessx*div.fromVert:			label
*accessx*div.fromHoriz:			lock
*accessx*mul.fromVert:			label
*accessx*mul.fromHoriz:			div
*accessx*minus.fromVert:		label
*accessx*minus.fromHoriz:		mul
*accessx*7.fromVert:			lock
*accessx*8.fromVert:			div
*accessx*8.fromHoriz:			7
*accessx*9.fromVert:			mul
*accessx*9.fromHoriz:			8
*accessx*plus.fromVert:			minus
*accessx*plus.fromHoriz:		9
*accessx*4.fromVert:			7
*accessx*5.fromVert:			8
*accessx*5.fromHoriz:			4
*accessx*6.fromVert:			9
*accessx*6.fromHoriz:			5
*accessx*1.fromVert:			4
*accessx*2.fromVert:			5
*accessx*2.fromHoriz:			1
*accessx*3.fromVert:			6
*accessx*3.fromHoriz:			2
*accessx*enter.fromVert:		plus
*accessx*enter.fromHoriz:		3
*accessx*0.fromVert:			2
*accessx*del.fromVert:			3
*accessx*del.fromHoriz:			0
*accessx.form.background:		gray80
*accessx*Label.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1
*accessx*label.font:			fixed
*accessx*Label.font:			-*-helvetica-bold-o-*-*-10-*-*-*-*-*-*-1
*accessx*Label.width:			50
*accessx*Label.height:			35
*accessx*label.width:			212
*accessx*label.height:			50
*accessx*plus.height:			74
*accessx*enter.height:			74
*accessx*0.width:			104
*accessx*lock.label:			Num\nLock
*accessx*div.label:			/\n
*accessx*mul.label:			*\n
*accessx*minus.label:			-\n
*accessx*7.label:			7\n
*accessx*8.label:			8\n
*accessx*9.label:			9\n
*accessx*plus.label:			+\n\ \n\ \n\ \n
*accessx*4.label:			4\n
*accessx*5.label:			5\n
*accessx*6.label:			6\n
*accessx*1.label:			1\n
*accessx*2.label:			2\n
*accessx*3.label:			3\n
*accessx*enter.label:			Enter\n\ \n\ \n\ \n
*accessx*0.label:			0\n
*accessx*del.label:			.\n\ \n
*accessx*label.displayList:\
foreground	white;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	gray20;\
lines		-1,0,0,0,0,-1
*accessx*label.label:\
If your mouse does not work, use\n\
the  numeric  keypad,  following\n\
the diagram bellow.
*accessx*div.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
font		-*-helvetica-bold-o-*-*-10-*-*-*-*-*-*-1;\
draw-string	4,30, "Button 1"
*accessx*mul.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
font		-*-helvetica-bold-o-*-*-10-*-*-*-*-*-*-1;\
draw-string	4,30, "Button 2"
*accessx*minus.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
font		-*-helvetica-bold-o-*-*-10-*-*-*-*-*-*-1;\
draw-string	4,30, "Button 3"
*accessx*7.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
fill-poly	20,20,20,30,30,20
*accessx*8.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
fill-poly	30,20,20,30,40,30
*accessx*9.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
fill-poly	20,20,30,30,30,20
*accessx*plus.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
font		-*-helvetica-bold-o-*-*-10-*-*-*-*-*-*-1;\
draw-string	9,46, "Double";\
draw-string	14,60, "Click"
*accessx*4.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
fill-poly	22,22,30,30,30,14
*accessx*5.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
font		-*-helvetica-bold-o-*-*-10*-*-*-*-*-*-1;\
draw-string	14,30, "Click"
*accessx*6.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
fill-poly	30,22,22,30,22,14
*accessx*1.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
fill-poly	20,20,20,30,30,30
*accessx*2.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
fill-poly	28,30,20,22,36,22
*accessx*3.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
fill-poly	20,30,30,30,30,20
*accessx*enter.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
font		-*-helvetica-bold-o-*-*-10-*-*-*-*-*-*-1;\
draw-string	9,46, "Toggle";\
draw-string	10,60, "Speed"
*accessx*0.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
font		-*-helvetica-bold-o-*-*-10-*-*-*-*-*-*-1;\
draw-string	26,30, "Button Lock"
*accessx*del.displayList:\
foreground	gray20;\
lines		+1,-1,-1,-1,-1,+1;\
foreground	white;\
lines		-1,0,0,0,0,-1;\
foreground	red;\
font		-*-helvetica-bold-o-*-*-8-*-*-*-*-*-*-1;\
draw-string	12,21, "Button";\
draw-string	9,30, "Release"

*panner.width:		100
*panner.height:		100

*help*text.properties:\
default?family=Times&weight=Medium&slant=R&pixelsize=12&registry=ISO8859&encoding=1,\
b?weight=Bold,\
strong?weight=Bold,\
i?slant=I,\
em?slant=I,\
address?slant=I,\
h1?pixelsize=24&weight=Bold,\
h2?pixelsize=20&weight=Bold,\
h3?pixelsize=17&weight=Bold,\
h4?pixelsize=14&weight=Bold,\
h5?pixelsize=12&weight=Bold,\
h6?pixelsize=10&weight=Bold,\
pre?family=Courier&pixelsize=10,\
kbd?family=Courier&pixelsize=10,\
code?family=Courier&pixelsize=10,\
samp?family=Courier&pixelsize=10,\
tt?family=Courier&pixelsize=10
*help*commands.min:	22
*help*commands.max:	22
*help*commands.showGrip:False
*help*wrap:		word
*help*commands.close.label:	Close
.xorgcfg.help.geometry:		320x369
.xorgcfg.help.minWidth:		320
.xorgcfg.help.maxWidth:		320
.xorgcfg.help.minHeight:	369
.xorgcfg.help.maxHeight:	369
.xorgcfg.help.translations:	#override \
<Message>WM_PROTOCOLS:	help-close()
*help*text.translations:\
<Key>Up:	scroll-one-line-down()\n\
<Key>Down:	scroll-one-line-up()\n\
<Key>Next:	next-page()\n\
<Key>Prior:	previous-page()\n\
<Key>space:	next-page()\n\
<Key>BackSpace:	previous-page()\n\
<Key>Home:	beginning-of-file()\n\
<Key>End:	end-of-file()
*help*text*displayCaret:	False
*help*text.leftMargin:		10
*help*text.rightMargin:		10

*help.helpDevices:\
<h2>Configuring devices</h2>\
You can arrange the icons pressing the <i>left mouse button</i> \
and dragging them.\
<p>\
To configure a device, press the <i>right mouse button</i> and choose the \
<tt>configure</tt> option.\

*help.helpScreen:\
<h2>Configuring screens</h2>\
You can drag the monitors to set the screen layout form <b>Xinerama</b> \
pressing the <i>left mouse button</i> and moving them.\
<p>\
Press the <i>right mouse button</i> to set configure or set options for \
the given screen.

*help.helpModeline:\
<h2>Configuring modelines</h2>\
<b><font size=+1 color=red>The controls here may damage your \
monitor.</font></b> \
<p>\
You can safelly skip this stage of the configuration process.\
<p>\
When not running in <b>Xinerama</b> mode, you can configure modelines \
for every configured monitor. Set the <b>Auto</b> toggle to see the changes \
while the <i>control buttons</i> are pressed.\
<p>\
Note that the <i>arrow</i> buttons are repeaters. Press they only once \
and they will show the changes in the <i>text labels</i>.\
<p>\
<font color=forestgreen>Press <b>ESC</b> if the monitor goes out of sync.</font>

*help.helpAccessX:\
<h2>Configuring accessx</h2>\
This interface is expected to allow customizing most of the <b>accessx</b> \
options.\
<p>\
Press in the <b>Enable ???</b> label to set specific options.\
<p>\
<font color=red>Note</font>: currently, if you press the \
<tt><b>Apply changes</b></tt> button there is no way to undo your changes.</font>


*Expert.geometry:				640x460
*Expert*vpane.min:				64
*Expert*vpane.max:				64
*Expert*vpane.showGrip:				False
*Expert*vpane.close.showGrip:			False
*Expert*vpane.close.min:			26
*Expert*vpane.close.max:			26
*panner.internalSpace:				1
*panner.shadowThickness:			0
*panner.shadowColor:				gray60
*panner.backgroundStipple:			black
*panner.borderColor:				gray40

*expert*tree.hSpace:				12
*expert*tree*Box.hSpace:			4
*expert*tree*vSpace:				4
*expert*tree*LogFile.Text.width:		192
*expert*tree*RgbPath.Text.width:		192
*expert*tree*ModulePath.Text.width:		192

*expert*tree*Form.defaultDistance:		4

*expert*tree.backgroundPixmap:			xlogo64?foreground=gray90&background=gray92
*expert*tree.foreground:			gray45

*expert*tree*Label.backgroundPixmap:		ParentRelative

*expert*tree*Box.backgroundPixmap:		gradient:vertical?dimension=3&start=gray85&end=gray95
*expert*tree*Box.borderWidth:			0
*expert*tree*Box.background:			gray85
*expert*Box.displayList:\
foreground	gray40;\
lines		1,-1,-1,-1,-1,1;\
foreground	white;\
lines		-1,0,0,0,0,-1

*expert*tree*Form.backgroundPixmap:		gradient:vertical?dimension=3&start=gray85&end=gray95
*expert*tree*Form.borderWidth:			0
*expert*tree*Form.background:			gray85
*expert*Form.displayList:\
foreground	gray40;\
lines		1,-1,-1,-1,-1,1;\
foreground	white;\
lines		-1,0,0,0,0,-1

*expert*tree*Text.width:			160

*expert*tree*fontpath.Text.width:		228
*expert*tree*fontpath.up.fromHoriz:		remove
*expert*tree*fontpath.down.fromHoriz:		up
*expert*tree*fontpath.value.fromVert:		remove
*expert*tree*fontpath.valueNew.fromVert:	new

*expert*tree*modulepath.Text.width:		228
*expert*tree*modulepath.value.fromVert:		remove
*expert*tree*modulepath.valueNew.fromVert:	new

*expert*tree*module.options.fromHoriz:		remove
*expert*tree*module.label.fromVert:		remove
*expert*tree*module.value.fromHoriz:		new
*expert*tree*module.value.width:		78

*expert*tree*video*Label.justify:		left
*expert*tree*video.options.fromHoriz:		remove
*expert*tree*video.adaptor.fromHoriz:		options
*expert*tree*video.vendorL.width:		78
*expert*tree*video.vendorL.label:		VendorName
*expert*tree*video.vendorL.fromVert:		remove
*expert*tree*video.vendor.fromVert:		remove
*expert*tree*video.vendor.fromHoriz:		vendorL
*expert*tree*video.boardL.width:		78
*expert*tree*video.boardL.label:		BoardName
*expert*tree*video.boardL.fromVert:		vendor
*expert*tree*video.board.fromVert:		vendor
*expert*tree*video.board.fromHoriz:		boardL
*expert*tree*video.busidL.width:		78
*expert*tree*video.busidL.label:		BusID
*expert*tree*video.busidL.fromVert:		board
*expert*tree*video.busid.fromVert:		board
*expert*tree*video.busid.fromHoriz:		busidL
*expert*tree*video.driverL.width:		78
*expert*tree*video.driverL.label:		Driver
*expert*tree*video.driverL.fromVert:		busid
*expert*tree*video.driver.fromVert:		busid
*expert*tree*video.driver.fromHoriz:		driverL
*expert*tree*video.value.fromHoriz:		new
*expert*tree*video.value.width:			78
*expert*tree*VideoPort.fromVert:		driver
*expert*tree*VideoPort.horizDistance:		120
*expert*tree*video.value.width:			149

*expert*tree*port.value.fromHoriz:		new
*expert*tree*port.options.fromHoriz:		remove
*expert*tree*port.label.fromVert:		remove
*expert*tree*port.value.width:			78

*expert*tree*modes.mode.fromHoriz:		remove
*expert*tree*modes.value.fromHoriz:		new
*expert*tree*modes.value.width:			78

*expert*tree*modeline.label.fromHoriz:		remove
*expert*tree*modeline.modeline.fromVert:	remove
*expert*tree*modeline.modeline.width:		480
*expert*tree*modeline.value.fromHoriz:		new
*expert*tree*modeline.value.width:		120
*expert*tree*modeline.modelineNew.fromVert:	value
*expert*tree*modeline.modelineNew.width:	480

*expert*tree*monitor.options.fromHoriz:		remove
*expert*tree*monitor.label.fromHoriz:		options

*expert*tree*monitor.Label.justify:		left
*expert*tree*monitor.Text.width:		120
*expert*tree*monitor.vendorL.width:		100
*expert*tree*monitor.vendorL.label:		VendorName
*expert*tree*monitor.vendorL.fromVert:		remove
*expert*tree*monitor.vendor.fromVert:		remove
*expert*tree*monitor.vendor.fromHoriz:		vendorL
*expert*tree*monitor.modelnameL.width:		100
*expert*tree*monitor.modelnameL.label:		ModelName
*expert*tree*monitor.modelnameL.fromVert:	vendor
*expert*tree*monitor.modelname.fromVert:	vendor
*expert*tree*monitor.modelname.fromHoriz:	modelnameL
*expert*tree*monitor.widthL.width:		100
*expert*tree*monitor.widthL.label:		Width (mm)
*expert*tree*monitor.widthL.fromVert:		modelname
*expert*tree*monitor.width.fromVert:		modelname
*expert*tree*monitor.width.fromHoriz:		widthL
*expert*tree*monitor.heightL.width:		100
*expert*tree*monitor.heightL.label:		Height (mm)
*expert*tree*monitor.heightL.fromVert:		width
*expert*tree*monitor.height.fromVert:		width
*expert*tree*monitor.height.fromHoriz:		heightL
*expert*tree*monitor.hsyncL.width:		100
*expert*tree*monitor.hsyncL.label:		Hsync
*expert*tree*monitor.hsyncL.fromVert:		heightL
*expert*tree*monitor.hsync.fromVert:		height
*expert*tree*monitor.hsync.fromHoriz:		hsyncL
*expert*tree*monitor.vrefreshL.width:		100
*expert*tree*monitor.vrefreshL.label:		Vrefresh
*expert*tree*monitor.vrefreshL.fromVert:	hsync
*expert*tree*monitor.vrefresh.fromVert:		hsync
*expert*tree*monitor.vrefresh.fromHoriz:	vrefreshL
*expert*tree*monitor.gammaRedL.width:		100
*expert*tree*monitor.gammaRedL.label:		Gamma (red)
*expert*tree*monitor.gammaRedL.fromVert:	vrefresh
*expert*tree*monitor.gammaRed.fromVert:		vrefresh
*expert*tree*monitor.gammaRed.fromHoriz:	gammaRedL
*expert*tree*monitor.gammaGreenL.width:		100
*expert*tree*monitor.gammaGreenL.label:		Gamma (green)
*expert*tree*monitor.gammaGreenL.fromVert:	gammaRed
*expert*tree*monitor.gammaGreen.fromVert:	gammaRed
*expert*tree*monitor.gammaGreen.fromHoriz:	gammaGreenL
*expert*tree*monitor.gammaBlueL.width:		100
*expert*tree*monitor.gammaBlueL.label:		Gamma (blue)
*expert*tree*monitor.gammaBlueL.fromVert:	gammaGreen
*expert*tree*monitor.gammaBlue.fromVert:	gammaGreen
*expert*tree*monitor.gammaBlue.fromHoriz:	gammaBlueL
*expert*tree*monitor.value.width:		191
*expert*tree*monitor.value.fromHoriz:		new

*expert*tree*device.Label.justify:		left
*expert*tree*device.options.fromHoriz:		remove
*expert*tree*device.label.fromHoriz:		options
*expert*tree*device.vendorL.label:		VendorName
*expert*tree*device.vendorL.width:		100
*expert*tree*device.vendorL.fromVert:		remove
*expert*tree*device.vendor.fromVert:		remove
*expert*tree*device.vendor.fromHoriz:		vendorL
*expert*tree*device.boardL.label:		BoardName
*expert*tree*device.boardL.width:		100
*expert*tree*device.boardL.fromVert:		vendor
*expert*tree*device.board.fromVert:		vendor
*expert*tree*device.board.fromHoriz:		boardL
*expert*tree*device.chipsetL.label:		Chipset
*expert*tree*device.chipsetL.width:		100
*expert*tree*device.chipsetL.fromVert:		board
*expert*tree*device.chipset.fromVert:		board
*expert*tree*device.chipset.fromHoriz:		chipsetL
*expert*tree*device.busidL.label:		BusID
*expert*tree*device.busidL.width:		100
*expert*tree*device.busidL.fromVert:		chipset
*expert*tree*device.busid.fromVert:		chipset
*expert*tree*device.busid.fromHoriz:		chipsetL
*expert*tree*device.cardL.label:		Card
*expert*tree*device.cardL.width:		100
*expert*tree*device.cardL.fromVert:		busid
*expert*tree*device.card.fromVert:		busid
*expert*tree*device.card.fromHoriz:		cardL
*expert*tree*device.driverL.label:		Driver
*expert*tree*device.driverL.width:		100
*expert*tree*device.driverL.fromVert:		card
*expert*tree*device.driver.fromVert:		card
*expert*tree*device.driver.fromHoriz:		driverL
*expert*tree*device.ramdacL.label:		Ramdac
*expert*tree*device.ramdacL.width:		100
*expert*tree*device.ramdacL.fromVert:		driverL
*expert*tree*device.ramdac.fromVert:		driver
*expert*tree*device.ramdac.fromHoriz:		ramdacL
*expert*tree*device.dacSpeedL.label:		DacSpeed
*expert*tree*device.dacSpeedL.width:		100
*expert*tree*device.dacSpeedL.fromVert:		ramdac
*expert*tree*device.dacSpeed.fromVert:		ramdac
*expert*tree*device.dacSpeed.fromHoriz:		dacSpeedL
*expert*tree*device.videoRamL.label:		VideoRam
*expert*tree*device.videoRamL.width:		100
*expert*tree*device.videoRamL.fromVert:		dacSpeed
*expert*tree*device.videoRam.fromVert:		dacSpeed
*expert*tree*device.videoRam.fromHoriz:		videoRamL
*expert*tree*device.textClockFreqL.label:	TextClockFreq
*expert*tree*device.textClockFreqL.width:	100
*expert*tree*device.textClockFreqL.fromVert:	videoRam
*expert*tree*device.textClockFreq.fromVert:	videoRam
*expert*tree*device.textClockFreq.fromHoriz:	textClockFreqL
*expert*tree*device.biosBaseL.label:		BiosBase
*expert*tree*device.biosBaseL.width:		100
*expert*tree*device.biosBaseL.fromVert:		textClockFreq
*expert*tree*device.biosBase.fromVert:		textClockFreq
*expert*tree*device.biosBase.fromHoriz:		biosBaseL
*expert*tree*device.memBaseL.label:		MemBase
*expert*tree*device.memBaseL.width:		100
*expert*tree*device.memBaseL.fromVert:		biosBase
*expert*tree*device.memBase.fromVert:		biosBase
*expert*tree*device.memBase.fromHoriz:		memBaseL
*expert*tree*device.ioBaseL.label:		IOBase
*expert*tree*device.ioBaseL.width:		100
*expert*tree*device.ioBaseL.fromVert:		memBase
*expert*tree*device.ioBase.fromVert:		memBase
*expert*tree*device.ioBase.fromHoriz:		ioBaseL
*expert*tree*device.clockChipL.label:		ClockChip
*expert*tree*device.clockChipL.width:		100
*expert*tree*device.clockChipL.fromVert:	ioBase
*expert*tree*device.clockChip.fromVert:		ioBase
*expert*tree*device.clockChip.fromHoriz:	clockChipL
*expert*tree*device.devClockL.label:		Clocks
*expert*tree*device.devClockL.width:		100
*expert*tree*device.devClockL.fromVert:		clockChip
*expert*tree*device.devClock.fromVert:		clockChip
*expert*tree*device.devClock.fromHoriz:		devClockL
*expert*tree*device.chipIdL.label:		ChipId
*expert*tree*device.chipIdL.width:		100
*expert*tree*device.chipIdL.fromVert:		devClock
*expert*tree*device.chipId.fromVert:		devClock
*expert*tree*device.chipId.fromHoriz:		chipIdL
*expert*tree*device.chipRevL.label:		ChipRev
*expert*tree*device.chipRevL.width:		100
*expert*tree*device.chipRevL.fromVert:		chipId
*expert*tree*device.chipRev.fromVert:		chipId
*expert*tree*device.chipRev.fromHoriz:		chipRevL
*expert*tree*device.irqL.label:			IRQ
*expert*tree*device.irqL.width:			100
*expert*tree*device.irqL.fromVert:		chipRev
*expert*tree*device.irq.fromVert:		chipRev
*expert*tree*device.irq.fromHoriz:		irqL
*expert*tree*device.screenL.label:		Screen
*expert*tree*device.screenL.width:		100
*expert*tree*device.screenL.fromVert:		irq
*expert*tree*device.screen.fromVert:		irq
*expert*tree*device.screen.fromHoriz:		screenL
*expert*tree*device.value.fromHoriz:		new

*expert*tree*screen.Label.justify:		left
*expert*tree*screen.options.fromHoriz:		remove
*expert*tree*screen.label.fromHoriz:		options
*expert*tree*screen.defaultDepthL.label:	DefaultDepth
*expert*tree*screen.defaultDepthL.width:	92
*expert*tree*screen.defaultDepthL.fromVert:	remove
*expert*tree*screen.defaultDepth.fromVert:	remove
*expert*tree*screen.defaultDepth.fromHoriz:	defaultDepthL
*expert*tree*screen.defaultBppL.label:		DefaultBpp
*expert*tree*screen.defaultBppL.width:		92
*expert*tree*screen.defaultBppL.fromVert:	defaultDepth
*expert*tree*screen.defaultBpp.fromVert:	defaultDepth
*expert*tree*screen.defaultBpp.fromHoriz:	defaultBppL
*expert*tree*screen.defaultFbBppL.label:	DefaultFbBpp
*expert*tree*screen.defaultFbBppL.width:	92
*expert*tree*screen.defaultFbBppL.fromVert:	defaultBpp
*expert*tree*screen.defaultFbBpp.fromVert:	defaultBpp
*expert*tree*screen.defaultFbBpp.fromHoriz:	defaultFbBppL
*expert*tree*screen.monitorL.label:		Monitor
*expert*tree*screen.monitorL.width:		92
*expert*tree*screen.monitorL.fromVert:		defaultFbBpp
*expert*tree*screen.monitor.fromVert:		defaultFbBpp
*expert*tree*screen.monitor.fromHoriz:		monitorL
*expert*tree*screen.deviceL.label:		Device
*expert*tree*screen.deviceL.width:		92
*expert*tree*screen.deviceL.fromVert:		monitor
*expert*tree*screen.device.fromVert:		monitor
*expert*tree*screen.device.fromHoriz:		deviceL
*expert*tree*screen.value.fromHoriz:		new

*expert*tree*adaptor.label.fromHoriz:		remove

*expert*tree*display.Label.width:		64
*expert*tree*display.Label.justify:		left
*expert*tree*display.options.fromHoriz:		remove
*expert*tree*display.viewportL.label:		Viewport
*expert*tree*display.viewportL.fromVert:	remove
*expert*tree*display.viewport.fromVert:		remove
*expert*tree*display.viewport.fromHoriz:	viewportL
*expert*tree*display.virtualL.label:		Virtual
*expert*tree*display.virtualL.fromVert:		viewport
*expert*tree*display.virtual.fromVert:		viewport
*expert*tree*display.virtual.fromHoriz:		virtualL
*expert*tree*display.depthL.label:		Depth
*expert*tree*display.depthL.fromVert:		virtual
*expert*tree*display.depth.fromVert:		virtual
*expert*tree*display.depth.fromHoriz:		depthL
*expert*tree*display.bppL.label:		FbBPP
*expert*tree*display.bppL.fromVert:		depth
*expert*tree*display.bpp.fromVert:		depth
*expert*tree*display.bpp.fromHoriz:		bppL
*expert*tree*display.visualL.label:		Visual
*expert*tree*display.visualL.fromVert:		bpp
*expert*tree*display.visual.fromVert:		bpp
*expert*tree*display.visual.fromHoriz:		visualL
*expert*tree*display.weightL.label:		Weight
*expert*tree*display.weightL.fromVert:		visual
*expert*tree*display.weight.fromVert:		visual
*expert*tree*display.weight.fromHoriz:		weightL
*expert*tree*display.blackL.label:		Black
*expert*tree*display.blackL.fromVert:		weight
*expert*tree*display.black.fromVert:		weight
*expert*tree*display.black.fromHoriz:		blackL
*expert*tree*display.whiteL.label:		White
*expert*tree*display.whiteL.fromVert:		black
*expert*tree*display.white.fromVert:		black
*expert*tree*display.white.fromHoriz:		whiteL

*expert*tree*mode.label.fromHoriz:		remove
*expert*tree*mode.value.fromHoriz:		new
*expert*tree*mode.value.width:			100

*expert*tree*input.options.fromHoriz:		remove
*expert*tree*input.label.fromHoriz:		options
*expert*tree*input.driverL.label:		Driver
*expert*tree*input.driverL.fromVert:		remove
*expert*tree*input.driver.fromVert:		remove
*expert*tree*input.driver.fromHoriz:		driverL
*expert*tree*input.value.fromHoriz:		new

*expert*tree*layout.options.fromHoriz:		remove
*expert*tree*layout.label.fromHoriz:		options
*expert*tree*layout.value.fromHoriz:		new

*expert*tree*adjacency.Text.width:		46
*expert*tree*adjacency.MenuButton.width:	122
*expert*tree*adjacency.label.fromHoriz:		remove
*expert*tree*adjacency.scrnumL.label:		Screen number
*expert*tree*adjacency.scrnumL.horizDistance:	50
*expert*tree*adjacency.scrnum.width:		32
*expert*tree*adjacency.scrnumL.fromVert:	remove
*expert*tree*adjacency.scrnum.fromVert:		remove
*expert*tree*adjacency.scrnum.fromHoriz:	scrnumL
*expert*tree*adjacency.above.label:		Above
*expert*tree*adjacency.above.fromVert:		scrnumL
*expert*tree*adjacency.above.vertDistance:	20
*expert*tree*adjacency.above.horizDistance:	96
*expert*tree*adjacency.below.label:		Below
*expert*tree*adjacency.below.horizDistance:	96
*expert*tree*adjacency.leftOf.label:		LeftOf
*expert*tree*adjacency.leftOf.fromVert:		above
*expert*tree*adjacency.screen.fromVert:		above
*expert*tree*adjacency.screen.fromHoriz:	leftOf
*expert*tree*adjacency.rightOf.label:		RightOf
*expert*tree*adjacency.rightOf.fromVert:	above
*expert*tree*adjacency.rightOf.fromHoriz:	screen
*expert*tree*adjacency.below.fromVert:		screen
*expert*tree*adjacency.relative.label:		Relative
*expert*tree*adjacency.relative.horizDistance:	53
*expert*tree*adjacency.relative.fromVert:	below
*expert*tree*adjacency.absolute.fromVert:	below
*expert*tree*adjacency*absolute.label:		Absolute
*expert*tree*adjacency*absolute.fromHoriz:	relative
*expert*tree*adjacency*adjxL.label:		X
*expert*tree*adjacency*adjxL.horizDistance:	42
*expert*tree*adjacency*adjxL.fromVert:		absolute
*expert*tree*adjacency*adjx.fromVert:		absolute
*expert*tree*adjacency*adjx.fromHoriz:		adjxL
*expert*tree*adjacency*adjyL.label:		Y
*expert*tree*adjacency*adjyL.horizDistance:	12
*expert*tree*adjacency*adjyL.fromVert:		absolute
*expert*tree*adjacency*adjyL.fromHoriz:		adjx
*expert*tree*adjacency*adjy.fromVert:		absolute
*expert*tree*adjacency*adjy.fromHoriz:		adjyL

*expert*tree*inputref.options.fromHoriz:	remove
*expert*tree*inputref.label.fromHoriz:		options

*expert*tree*vendor.Text.width:			100
*expert*tree*vendor.options.fromHoriz:		remove
*expert*tree*vendor.label.fromHoriz:		options
*expert*tree*vendor.value.fromHoriz:		new

*expert*tree*vendorSub.Text.width:		140
*expert*tree*vendorSub.options.fromHoriz:	remove
*expert*tree*vendorSub.label.fromHoriz:		options
*expert*tree*vendorSub.nameL.label:		Name
*expert*tree*vendorSub.nameL.fromVert:		remove
*expert*tree*vendorSub.name.fromVert:		remove
*expert*tree*vendorSub.name.fromHoriz:		nameL
*expert*tree*vendorSub.value.fromHoriz:		new

*expert*tree*dri.Text.width:			100
*expert*tree*dri.Label.width:			78
*expert*tree*dri.Label.justify:			left
*expert*tree*dri.nameL.label:			Group name
*expert*tree*dri.name.fromHoriz:		nameL
*expert*tree*dri.groupL.label:			Group
*expert*tree*dri.groupL.fromVert:		name
*expert*tree*dri.group.fromVert:		name
*expert*tree*dri.group.fromHoriz:		groupL
*expert*tree*dri.modeL.label:			Mode
*expert*tree*dri.modeL.fromVert:		group
*expert*tree*dri.mode.fromVert:			group
*expert*tree*dri.mode.fromHoriz:		modeL

*expert*tree*buffers.Label.width:		50
*expert*tree*buffers.Text.width:		100
*expert*tree*buffers.countL.label:		Count
*expert*tree*buffers.countL.fromVert:		remove
*expert*tree*buffers.count.fromVert:		remove
*expert*tree*buffers.count.fromHoriz:		countL
*expert*tree*buffers.sizeL.label:		Size
*expert*tree*buffers.sizeL.fromVert:		count
*expert*tree*buffers.size.fromVert:		count
*expert*tree*buffers.size.fromHoriz:		sizeL
*expert*tree*buffers.flagsL.label:		Flags
*expert*tree*buffers.flagsL.fromVert:		size
*expert*tree*buffers.flags.fromVert:		size
*expert*tree*buffers.flags.fromHoriz:		flagsL
*Expert*close.label:				Close
