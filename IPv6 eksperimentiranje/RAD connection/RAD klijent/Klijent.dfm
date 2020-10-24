object Form2: TForm2
  Left = 0
  Top = 0
  Caption = 'Form2'
  ClientHeight = 299
  ClientWidth = 635
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBox1: TGroupBox
    Left = 8
    Top = 208
    Width = 81
    Height = 83
    Caption = 'Konekcija'
    TabOrder = 0
    object Button2: TButton
      Left = 3
      Top = 49
      Width = 75
      Height = 25
      Caption = 'Zaustavi'
      TabOrder = 0
      OnClick = Button2Click
    end
    object Button1: TButton
      Left = 3
      Top = 18
      Width = 75
      Height = 25
      Caption = 'Pove'#382'i'
      TabOrder = 1
      OnClick = Button1Click
    end
  end
  object memIspis: TMemo
    Left = 274
    Top = 6
    Width = 353
    Height = 285
    Color = clBlack
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clYellow
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    Lines.Strings = (
      'memIspis')
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 1
  end
  object RGTest: TRadioGroup
    Left = 95
    Top = 208
    Width = 138
    Height = 83
    Caption = 'RG test'
    Columns = 2
    ItemIndex = 0
    Items.Strings = (
      'RB 1'
      'RB 2'
      'RB 3'
      'RB 4')
    TabOrder = 2
    OnClick = RGTestClick
  end
  object MainMenu1: TMainMenu
    Left = 608
    Top = 65528
    object Server1: TMenuItem
      Caption = 'Server'
      object Pokreni1: TMenuItem
        Caption = 'Pokreni'
        OnClick = Button3Click
      end
      object Povei1: TMenuItem
        Caption = 'Pove'#382'i'
        OnClick = Button1Click
      end
      object Zaustavi1: TMenuItem
        Caption = 'Zaustavi'
        OnClick = Button2Click
      end
    end
  end
end
