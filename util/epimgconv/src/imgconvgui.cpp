
// epimgconv: Enterprise 128 image converter utility
// Copyright (C) 2008-2016 Istvan Varga <istvanv@users.sourceforge.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// The Enterprise 128 program files generated by this utility are not covered
// by the GNU General Public License, and can be used, modified, and
// distributed without any restrictions.

#include "epimgconv.hpp"
#include "imageconv.hpp"
#include "imgwrite.hpp"
#include "guicolor.hpp"

#include <vector>
#include <map>

#include <FL/Fl.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Shared_Image.H>

void Ep128ImgConvGUI::init_()
{
  display = (Ep128ImgConvGUI_Display *) 0;
  videoMemory = (Ep128::Memory *) 0;
  nick = (Ep128ImgConvGUI_Nick *) 0;
  imageFileData = (unsigned char *) 0;
  browseFileWindow = (Fl_File_Chooser *) 0;
  imageData = (Ep128ImgConv::ImageData *) 0;
  try {
    display = new Ep128ImgConvGUI_Display(*this, 4, 35, 872, 576, "", false);
    videoMemory = new Ep128::Memory();
    nick = new Ep128ImgConvGUI_Nick(*display, *videoMemory);
    emulatorWindow = display;
    imageFileData = new unsigned char[872 * 576 * 3];
    unsigned char r = 0x00;
    unsigned char g = 0x00;
    unsigned char b = 0x00;
    Fl::get_color(Fl_Color(48), r, g, b);
    for (int i = 0; i < (872 * 576 * 3); i += 3) {
      imageFileData[i] = r;
      imageFileData[i + 1] = g;
      imageFileData[i + 2] = b;
    }
    imageFileName = "";
    busyFlag = false;
    stopFlag = false;
    previewEnabled = false;
    fileChangedFlag = false;
    fileNotSavedFlag = false;
    confirmStatus = false;
    browseFileWindow =
        new Fl_File_Chooser("", "*", Fl_File_Chooser::SINGLE, "");
    imageFileDirectory = "";
    configDirectory = "";
    outputFileDirectory = "";
    guiConfig.createKey("imageFileDirectory", imageFileDirectory);
    guiConfig.createKey("configDirectory", configDirectory);
    guiConfig.createKey("outputFileDirectory", outputFileDirectory);
    emulationTimer.reset();
    for (int i = 0; true; i++) {
      const char  *s = Ep128ImgConv::ImageConvConfig::getVideoModeName(i);
      if (!s)
        break;
      conversionTypeValuator->add(s);
    }
    for (int i = 0; true; i++) {
      const char  *s = Ep128ImgConv::ImageConvConfig::getDitherName(i);
      if (!s)
        break;
      ditherTypeValuator->add(s);
    }
    for (int i = 0; true; i++) {
      const char  *s = Ep128ImgConv::ImageConvConfig::getOutputFormatName(i);
      if (!s)
        break;
      outputFormatValuator->add(s);
    }
    Fl::add_check(&fltkCheckCallback, (void *) this);
  }
  catch (...) {
    if (browseFileWindow) {
      delete browseFileWindow;
      browseFileWindow = (Fl_File_Chooser *) 0;
    }
    if (imageFileData) {
      delete[] imageFileData;
      imageFileData = (unsigned char *) 0;
    }
    if (nick) {
      delete nick;
      nick = (Ep128ImgConvGUI_Nick *) 0;
    }
    if (videoMemory) {
      delete videoMemory;
      videoMemory = (Ep128::Memory *) 0;
    }
    if (display) {
      delete display;
      display = (Ep128ImgConvGUI_Display *) 0;
    }
    throw;
  }
}

void Ep128ImgConvGUI::updateDisplay(double t)
{
  Fl::wait(t);
}

bool Ep128ImgConvGUI::confirmMessage(const char *msg)
{
  while (confirmMessageWindow->shown())
    updateDisplay();
  confirmStatus = false;
  if (msg)
    confirmMessageText->copy_label(msg);
  else
    confirmMessageText->label("");
  confirmMessageWindow->set_modal();
  confirmMessageWindow->show();
  do {
    updateDisplay();
  } while (confirmMessageWindow->shown());
  confirmMessageText->label("");
  return confirmStatus;
}

void Ep128ImgConvGUI::errorMessage(const char *msg)
{
  while (errorMessageWindow->shown())
    updateDisplay();
  if (msg)
    errorMessageText->copy_label(msg);
  else
    errorMessageText->label("");
  errorMessageWindow->set_modal();
  errorMessageWindow->show();
  do {
    updateDisplay();
  } while (errorMessageWindow->shown());
  errorMessageText->label("");
}

bool Ep128ImgConvGUI::browseFile(std::string& fileName, std::string& dirName,
                                 const char *pattern, int type,
                                 const char *title)
{
  while (browseFileWindow->shown())
    updateDisplay();
  browseFileWindow->directory(dirName.c_str());
  browseFileWindow->filter(pattern);
  browseFileWindow->type(type);
  browseFileWindow->label(title);
  browseFileWindow->show();
  try {
    std::string tmp = dirName;
    if (tmp.length() < 1 ||
        (tmp[tmp.length() - 1] != '/' && tmp[tmp.length() - 1] != '\\')) {
#ifdef WIN32
      tmp += '\\';
#else
      tmp += '/';
#endif
    }
    size_t  n = fileName.length();
    while (n > 0) {
      if (fileName[n - 1] == '/' || fileName[n - 1] == '\\')
        break;
      n--;
    }
    for (size_t i = n; i < fileName.length(); i++)
      tmp += fileName[i];
    browseFileWindow->value(tmp.c_str());
  }
  catch (...) {
    browseFileWindow->value("");
  }
  do {
    updateDisplay();
  } while (browseFileWindow->shown());
  try {
    fileName.clear();
    if (browseFileWindow->value() != (char *) 0) {
      fileName = browseFileWindow->value();
      Ep128Emu::stripString(fileName);
      std::string tmp;
      Ep128Emu::splitPath(fileName, dirName, tmp);
      return true;
    }
  }
  catch (std::exception& e) {
    fileName.clear();
    errorMessage(e.what());
  }
  return false;
}

void Ep128ImgConvGUI::applyConfigurationChanges()
{
  if (busyFlag || !(fileChangedFlag || config.configChangeFlag))
    return;
  try {
    if (imageData) {
      delete imageData;
      imageData = (Ep128ImgConv::ImageData *) 0;
    }
    updateConfigWindow();
    if (imageFileName.empty()) {
      // no input file, nothing to do
      fileChangedFlag = false;
      config.clearConfigurationChangeFlag();
      return;
    }
    setBusyFlag(true);
    Ep128ImgConv::ImageData *imgData =
        Ep128ImgConv::convertImage(imageFileName.c_str(), config,
                                   &progressMessageCallback,
                                   &progressPercentageCallback, (void *) this);
    if (imgData) {
      imageData = imgData;
      fileChangedFlag = false;
      fileNotSavedFlag = true;
      nick->loadImage(*imgData, ((config.conversionType % 10) >= 7));
    }
    else {
      fileChangedFlag = true;
      fileNotSavedFlag = false;
      nick->reset();
    }
    setBusyFlag(false);
  }
  catch (std::exception& e) {
    setBusyFlag(false);
    fileChangedFlag = true;
    fileNotSavedFlag = false;
    nick->reset();
    errorMessage(e.what());
  }
}

void Ep128ImgConvGUI::setWidgetColors(Fl_Widget *epWidget, Fl_Widget *tvcWidget,
                                      int c)
{
  for (int i = 0; i < 2; i++) {
    Fl_Widget *o = (i == 0 ? epWidget : tvcWidget);
    if (!o)
      continue;
    if (c < 0) {
      o->color(Fl_Color(48));
      continue;
    }
    int     ri = 0;
    int     gi = 0;
    int     bi = 0;
    if (i == 0) {
      Ep128ImgConv::convertEPColorToRGB(c, ri, gi, bi);
      ri = (ri * 255 + 3) / 7;
      gi = (gi * 255 + 3) / 7;
      bi = (bi * 255 + 1) / 3;
    }
    else {
      int     tmp = (!(c & 0xC0) ? ((4 * 255 + 3) / 7) : 255);
      ri = (!(c & 0x0C) ? 0 : tmp);
      gi = (!(c & 0x30) ? 0 : tmp);
      bi = (!(c & 0x03) ? 0 : tmp);
    }
    unsigned int  tmp = ((unsigned int) ri << 24)
                        | ((unsigned int) gi << 16)
                        | ((unsigned int) bi << 8);
    if (tmp != 0U)
      o->color(Fl_Color(tmp));
    else
      o->color(FL_BLACK);
    o->redraw();
  }
}

void Ep128ImgConvGUI::updateConfigWindow()
{
  try {
    int     conversionType = config["conversionType"];
    int     outputFormat = config["outputFormat"];
    int     ditherType = config["ditherType"];
    if (((conversionType % 10) >= 7) != (outputFormat >= 11)) {
      outputFormat = ((conversionType % 10) < 7 ? 1 : 11);
      config["outputFormat"] = outputFormat;
    }
    conversionTypeValuator->value(conversionType);
    imageWidthValuator->value(double(int(config["width"])));
    imageHeightValuator->value(double(int(config["height"])));
    scaleXValuator->value(double(config["scaleX"]));
    scaleYValuator->value(double(config["scaleY"]));
    offsetXValuator->value(double(config["offsetX"]));
    offsetYValuator->value(double(config["offsetY"]));
    yMinValuator->value(double(config["yMin"]));
    yMaxValuator->value(double(config["yMax"]));
    saturationMultValuator->value(double(config["colorSaturationMult"]));
    gammaCorrectionValuator->value(double(config["gammaCorrection"]));
    if (outputFormat >= 6 && outputFormat <= 10) {
      paletteResolutionValuator->deactivate();
      paletteResolutionValuator->value(0);
    }
    else {
      paletteResolutionValuator->value(config["paletteResolution"]);
      paletteResolutionValuator->activate();
    }
    ditherTypeValuator->value(ditherType);
    if (ditherType >= 4) {
      ditherDiffusionValuator->deactivate();
      ditherDiffusionValuator->value(0.0);
    }
    else {
      ditherDiffusionValuator->value(double(config["ditherDiffusion"]));
      ditherDiffusionValuator->activate();
    }
    if ((conversionType % 10) == 5 || (conversionType % 10) == 9) {
      conversionQualityValuator->deactivate();
      conversionQualityValuator->value(9.0);
    }
    else {
      conversionQualityValuator->value(
          double(int(config["conversionQuality"])));
      conversionQualityValuator->activate();
    }
    borderColorValuator->value(double(int(config["borderColor"])));
    setWidgetColors(borderColorDisplay, borderColorTVCDisplay,
                    int(config["borderColor"]));
    outputFormatValuator->value(outputFormat);
    if ((outputFormat >= 2 && outputFormat <= 6) ||
        (outputFormat >= 13 && outputFormat <= 16)) {
      compressionLevelValuator->value(double(config["compressionLevel"]));
      compressionLevelValuator->activate();
    }
    else {
      compressionLevelValuator->deactivate();
      compressionLevelValuator->value(5.0);
    }
    // advanced settings tab
    noInterpolationValuator->value(int(bool(config["noInterpolation"])));
    scaleModeValuator->value(int(bool(config["scaleMode"])));
    if (outputFormat) {
      noCompressValuator->deactivate();
      noCompressValuator->value(0);
    }
    else {
      noCompressValuator->value(int(bool(config["noCompress"])));
      noCompressValuator->activate();
    }
    colorErrorScaleValuator->value(double(config["colorErrorScale"]));
    color0Valuator->value(double(int(config["color0"])));
    setWidgetColors(color0Display, color0TVCDisplay, int(config["color0"]));
    color1Valuator->value(double(int(config["color1"])));
    setWidgetColors(color1Display, color1TVCDisplay, int(config["color1"]));
    color2Valuator->value(double(int(config["color2"])));
    setWidgetColors(color2Display, color2TVCDisplay, int(config["color2"]));
    color3Valuator->value(double(int(config["color3"])));
    setWidgetColors(color3Display, color3TVCDisplay, int(config["color3"]));
    color4Valuator->value(double(int(config["color4"])));
    setWidgetColors(color4Display, color4TVCDisplay, int(config["color4"]));
    color5Valuator->value(double(int(config["color5"])));
    setWidgetColors(color5Display, color5TVCDisplay, int(config["color5"]));
    color6Valuator->value(double(int(config["color6"])));
    setWidgetColors(color6Display, color6TVCDisplay, int(config["color6"]));
    color7Valuator->value(double(int(config["color7"])));
    setWidgetColors(color7Display, color7TVCDisplay, int(config["color7"]));
    {
      int     bias = int(config["fixBias"]);
      fixBiasValuator->value(double(bias));
      setWidgetColors(fixBiasDisplay0, (Fl_Widget *) 0, bias * 8);
      setWidgetColors(fixBiasDisplay1, (Fl_Widget *) 0, bias * 8 + 1);
      setWidgetColors(fixBiasDisplay2, (Fl_Widget *) 0, bias * 8 + 2);
      setWidgetColors(fixBiasDisplay3, (Fl_Widget *) 0, bias * 8 + 3);
      setWidgetColors(fixBiasDisplay4, (Fl_Widget *) 0, bias * 8 + 4);
      setWidgetColors(fixBiasDisplay5, (Fl_Widget *) 0, bias * 8 + 5);
      setWidgetColors(fixBiasDisplay6, (Fl_Widget *) 0, bias * 8 + 6);
      setWidgetColors(fixBiasDisplay7, (Fl_Widget *) 0, bias * 8 + 7);
    }
  }
  catch (std::exception& e) {
    errorMessage(e.what());
  }
}

void Ep128ImgConvGUI::openImageFile()
{
  if (busyFlag)
    return;
  try {
    {
      std::string tmp = imageFileName;
      if (!browseFile(tmp, imageFileDirectory,
                      "Image files (*.{bmp,jpg,png,gif,xpm,ppm})",
                      Fl_File_Chooser::SINGLE,
                      "Open image file")) {
        tmp = "";
      }
      if (tmp == "")
        return;
      imageFileName = tmp;
    }
    fileChangedFlag = true;
    updateImageDisplay();
  }
  catch (std::exception& e) {
    setBusyFlag(false);
    errorMessage(e.what());
  }
  if (fileChangedFlag && previewEnabled)
    applyConfigurationChanges();
}

void Ep128ImgConvGUI::updateImageDisplay()
{
  try {
    if (imageFileName.length() < 1)
      return;
    Ep128ImgConv::YUVImageConverter imgConv;
    imgConv.setImageSize(872, 576);
    imgConv.setPixelAspectRatio(1.0f);
    unsigned char r = 0x00;
    unsigned char g = 0x00;
    unsigned char b = 0x00;
    Fl::get_color(Fl_Color(48), r, g, b);
    float   y = 0.0f;
    float   u = 0.0f;
    float   v = 0.0f;
    Ep128ImgConv::rgbToYUV(y, u, v,
                           float(int(r)) / 255.0f,
                           float(int(g)) / 255.0f,
                           float(int(b)) / 255.0f);
    imgConv.setBorderColor(y, u, v);
    imgConv.setPixelStoreCallback(&pixelStoreCallback, (void *) this);
    imgConv.setProgressMessageCallback(&progressMessageCallback,
                                       (void *) this);
    imgConv.setProgressPercentageCallback(&progressPercentageCallback,
                                          (void *) this);
    setBusyFlag(true);
    if (!imgConv.convertImageFile(imageFileName.c_str())) {
      fileChangedFlag = false;
      imageFileName = "";
    }
    setBusyFlag(false);
  }
  catch (std::exception& e) {
    setBusyFlag(false);
    errorMessage(e.what());
  }
}

static const char *outputFormatTable[17 * 3] = {
  // extension, filter, title
  ".com", "Enterprise programs (*.com)", "Save Enterprise program",
  ".pic", "IVIEW images (*.pic)", "Save IVIEW image",
  ".pic", "IVIEW images (*.pic)", "Save IVIEW image",
  ".pic", "IVIEW images (*.pic)", "Save IVIEW image",
  ".pic", "IVIEW images (*.pic)", "Save IVIEW image",
  ".pic", "IVIEW images (*.pic)", "Save IVIEW image",
  ".crf", "Agsys CRF images (*.crf)", "Save Agsys CRF image",
  ".scr", "ZozoTools VL/VS images (*.scr)", "Save ZozoTools VL/VS image",
  ".pic", "PaintBox images (*.pic)", "Save PaintBox image",
  ".pic", "Zaxial images (*.pic)", "Save Zaxial image",
  ".bin", "Raw image data (*.bin)", "Save raw Enterprise image data",
  ".kep", "TVC KEP files (*.kep)", "Save TVC KEP file",
  ".kep", "TVC KEP files (*.kep)", "Save TVC KEP file",
  ".kep", "TVC KEP files (*.kep)", "Save TVC KEP file",
  ".kep", "TVC KEP files (*.kep)", "Save TVC KEP file",
  ".kep", "TVC KEP files (*.kep)", "Save TVC KEP file",
  ".kep", "TVC KEP files (*.kep)", "Save TVC KEP file"
};

void Ep128ImgConvGUI::saveImageFile()
{
  if (!busyFlag) {
    applyConfigurationChanges();
    if (!imageData) {
      fileNotSavedFlag = false;
      return;
    }
    try {
      std::string outFileName = imageFileName;
      if (outFileName.length() > 4) {
        const char  *s = outFileName.c_str() + (outFileName.length() - 4);
        uint32_t    tmp = (uint32_t((unsigned char) s[0]) << 24)
                          | (uint32_t((unsigned char) s[1]) << 16)
                          | (uint32_t((unsigned char) s[2]) << 8)
                          | uint32_t((unsigned char) s[3]);
        tmp = tmp & 0xFFDFDFDFU;
        if (tmp == 0x2E4A5047U || tmp == 0x2E504E47U ||     // ".JPG" || ".PNG"
            tmp == 0x2E424D50U || tmp == 0x2E474946U ||     // ".BMP" || ".GIF"
            tmp == 0x2E58504DU || tmp == 0x2E50504DU) {     // ".XPM" || ".PPM"
          outFileName.resize(outFileName.length() - 4);
        }
      }
      int     outputFormat = int(config["outputFileFormat"]);
      if (!(outputFormat >= 0 && outputFormat <= 16))
        outputFormat = 10;              // default to raw image data
      outFileName += outputFormatTable[outputFormat * 3];
      if (!browseFile(outFileName, outputFileDirectory,
                      outputFormatTable[outputFormat * 3 + 1],
                      Fl_File_Chooser::CREATE,
                      outputFormatTable[outputFormat * 3 + 2])) {
        outFileName = "";
      }
      if (outFileName != "") {
        setBusyFlag(true);
        if (Ep128ImgConv::writeConvertedImageFile(
                outFileName.c_str(), *imageData, config.getOutputFormat(),
                config["noCompress"], &progressMessageCallback,
                &progressPercentageCallback, (void *) this)) {
          fileNotSavedFlag = false;
        }
        setBusyFlag(false);
      }
    }
    catch (std::exception& e) {
      setBusyFlag(false);
      errorMessage(e.what());
    }
  }
}

void Ep128ImgConvGUI::setBusyFlag(bool newState)
{
  if (newState != busyFlag) {
    busyFlag = newState;
    stopFlag = false;
    if (busyFlag) {
      openImageButton->deactivate();
      saveImageButton->deactivate();
      stopButton->activate();
    }
    else {
      openImageButton->activate();
      saveImageButton->activate();
      stopButton->deactivate();
    }
    emulatorWindow->redraw();
  }
}

void Ep128ImgConvGUI::fltkCheckCallback(void *userData)
{
  Ep128ImgConvGUI&  this_ = *(reinterpret_cast<Ep128ImgConvGUI *>(userData));
  try {
    if (this_.display->checkEvents())
      this_.emulatorWindow->redraw();
  }
  catch (...) {
  }
}

void Ep128ImgConvGUI::progressMessageCallback(void *userData, const char *msg)
{
  Ep128ImgConvGUI&  this_ = *(reinterpret_cast<Ep128ImgConvGUI *>(userData));
  if (msg == (char *) 0 || msg[0] == '\0') {
    msg = "";
    this_.progressDisplay->label(msg);
  }
  else
    this_.progressDisplay->copy_label(msg);
  if (msg[0] == '\0')
    this_.progressDisplay->value(0.0f);
}

bool Ep128ImgConvGUI::progressPercentageCallback(void *userData, int n)
{
  Ep128ImgConvGUI&  this_ = *(reinterpret_cast<Ep128ImgConvGUI *>(userData));
  this_.progressDisplay->value(float(n));
  this_.updateDisplay(0.0);
  return !(this_.stopFlag);
}

void Ep128ImgConvGUI::pixelStoreCallback(void *userData, int xc, int yc,
                                         float y, float u, float v)
{
  Ep128ImgConvGUI&  this_ = *(reinterpret_cast<Ep128ImgConvGUI *>(userData));
  float   r = 0.0f;
  float   g = 0.0f;
  float   b = 0.0f;
  Ep128ImgConv::yuvToRGB(r, g, b, y, u, v);
  Ep128ImgConv::limitRGBColor(r, g, b);
  int     ri = int(r * 255.0f + 0.5f);
  int     gi = int(g * 255.0f + 0.5f);
  int     bi = int(b * 255.0f + 0.5f);
  int     offs = ((yc * 872) + xc) * 3;
  this_.imageFileData[offs] = (unsigned char) ri;
  this_.imageFileData[offs + 1] = (unsigned char) gi;
  this_.imageFileData[offs + 2] = (unsigned char) bi;
}

void Ep128ImgConvGUI::run()
{
  Ep128Emu::setWindowIcon(aboutWindow, 10);
  Ep128Emu::setWindowIcon(confirmMessageWindow, 11);
  Ep128Emu::setWindowIcon(errorMessageWindow, 12);
  aboutWindowText->value(
      "epimgconv: Enterprise 128 image converter utility\n"
      "Copyright (C) 2008-2016 by Istvan Varga; ep128emu is copyright\n"
      "(C) 2003-2016 by Istvan Varga <istvanv@users.sourceforge.net>\n"
      "\n"
      "This program is free software; you can redistribute it and/or\n"
      "modify it under the terms of the GNU General Public License as\n"
      "published by the Free Software Foundation; either version 2 of\n"
      "the License, or (at your option) any later version.\n"
      "\n"
      "This program is distributed in the hope that it will be\n"
      "useful, but WITHOUT ANY WARRANTY; without even the implied\n"
      "warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR\n"
      "PURPOSE.  See the GNU General Public License for more details.\n"
      "\n"
      "You should have received a copy of the GNU General Public\n"
      "License along with this program; if not, write to the Free\n"
      "Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,\n"
      "MA  02111-1307  USA\n"
      "\n"
      "The Enterprise 128 program files generated by this utility\n"
      "are not covered by the GNU General Public License, and can be\n"
      "used, modified, and distributed without any restrictions.");
  try {
    // load configuration
    Ep128Emu::File  f("epimggui.dat", true);
    guiConfig.registerChunkType(f);
    f.processAllChunks();
  }
  catch (...) {
  }
  nick->reset();
  emulatorWindow->resize(4, 35, 872, 576);
  mainWindow->show();
  emulatorWindow->show();
  do {
    updateDisplay();
  } while (mainWindow->shown());
  try {
    // save configuration
    Ep128Emu::File  f;
    guiConfig.saveState(f);
    f.writeFile("epimggui.dat", true);
  }
  catch (...) {
  }
}
