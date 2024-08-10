object Form14: TForm14
  Left = 0
  Top = 0
  Caption = 'Form14'
  ClientHeight = 477
  ClientWidth = 1834
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Verdana'
  Font.Style = []
  TextHeight = 14
  object MemoDebug: TMemo
    Left = 632
    Top = 68
    Width = 473
    Height = 389
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssBoth
    TabOrder = 0
  end
  object Button1: TButton
    Left = 368
    Top = 32
    Width = 75
    Height = 25
    Caption = 'Quit'
    TabOrder = 1
    OnClick = Button1Click
  end
  object BN_Start: TButton
    Left = 56
    Top = 24
    Width = 75
    Height = 25
    Caption = 'Stop'
    TabOrder = 2
    OnClick = BN_StartClick
  end
  object Memo_ICAO: TMemo
    Left = 1111
    Top = 68
    Width = 715
    Height = 389
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssBoth
    TabOrder = 3
  end
  object MemoRaw: TMemo
    Left = 8
    Top = 68
    Width = 609
    Height = 389
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssBoth
    TabOrder = 4
  end
  object IdUDPServer1: TIdUDPServer
    Active = True
    Bindings = <>
    DefaultPort = 31012
    ReuseSocket = rsTrue
    OnUDPRead = IdUDPServer1UDPRead
    Left = 776
    Top = 65528
  end
  object Timer1: TTimer
    OnTimer = Timer1Timer
    Left = 896
    Top = 24
  end
end
