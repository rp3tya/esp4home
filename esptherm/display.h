#include "esphome.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "XPT2046_Touchscreen.h"
#include "DigitalDisplay12pt7b.h"
#include "DigitalDisplay24pt7b.h"
#include "DigitalDisplay60pt7b.h"
#include "Ticons12pt7b.h"

#define TEMP_MIN 10.0
#define TEMP_MAX 50.0
#define TEMP_STEP 0.5

enum OperationMode : uint8_t {
	HEAT = 0,
	COOL = 1
};

enum ProgramMode : uint8_t {
	AUTO = 0,
	MANU = 1,
	HOLD = 2
};

enum DisplayPage : uint8_t { 
  TEMP  = 0,
  CLOCK = 1
};

//*****************************************************************************
class Page {
protected:
  class Element {
    uint16_t locx;
    uint16_t locy;
    const GFXfont *font;
    uint8_t size; 
    void (*provide)(char*, int*);
    int color;
    char content[2];
  public:
    Element(const GFXfont *f, uint16_t fs, uint16_t cx, uint16_t cy, void (*cp)(char*, int*) ) {
      font = f;
      size = fs;
      locx = cx;
      locy = cy;
      provide = cp;
    }
    void reset() {
      color = -1;
    }
    void paint2(Adafruit_ILI9341 *tft) {
      int acolor = color;
      char acontent[strlen(content)+1];
      strcpy(acontent, content);
      // update content
      provide(content, &color);
      if (strcmp(acontent, content) != 0 || acolor != color) {
				tft->setFont(font);
				tft->setTextSize(size);
				tft->setCursor(locx, locy);
				tft->setTextColor(ILI9341_BLACK);
				tft->print(acontent);
				tft->setCursor(locx, locy);
				tft->setTextColor(color);
				tft->print(content);
      }
    }
  };
  std::vector<Element*> elements;
public:
  void addElement(const GFXfont *f, uint16_t fs, uint16_t cx, uint16_t cy, void (*cp)(char*, int*) ) {
    Element *elem = new Element(f, fs, cx, cy, cp);
    elements.reserve(elements.size() + 1);
    elements.push_back(elem);
  }
	void show(Adafruit_ILI9341 *tft) {
		tft->fillScreen(ILI9341_BLACK);
    for (int i=0; i<elements.size(); i++) {
      elements[i]->reset();
      elements[i]->paint2(tft);
    }
  }
	void update(Adafruit_ILI9341 *tft) {
    for (int i=0; i<elements.size(); i++) {
      elements[i]->paint2(tft);
    }
  }
	virtual int click(TS_Point *p) {
    return -1;
  }
};

class PageTemp : public Page {
public:
  PageTemp() : Page() {
    addElement(&ticons12pt7b, 1, 6, 30, [](char* str, int* cc){
      sprintf(str, "%s", "b");
      *cc = term_mode->value()==OperationMode::HEAT ? ILI9341_CYAN : ILI9341_DARKGREY;
    });
    addElement(&ticons12pt7b, 1, 36, 30, [](char* str, int* cc){
      sprintf(str, "%s", "a");
      *cc = term_mode->value()==OperationMode::COOL ? ILI9341_YELLOW : ILI9341_DARKGREY;
    });
    addElement(&ticons12pt7b, 1, 108, 30, [](char* str, int* cc){
      sprintf(str, "%s", "c");
      *cc = term_flag->value() ? (term_mode->value()==OperationMode::COOL ? ILI9341_CYAN : ILI9341_ORANGE) : ILI9341_DARKGREY;
    });
    addElement(&ticons12pt7b, 1, 180, 30, [](char* str, int* cc){
      sprintf(str, "%s", "f");
      *cc = term_prog->value()==ProgramMode::AUTO || term_prog->value()==ProgramMode::MANU ? ILI9341_GREENYELLOW : ILI9341_DARKGREY;
    });
    addElement(&ticons12pt7b, 1, 210, 30, [](char* str, int* cc){
      sprintf(str, "%s", "g");
      *cc = term_prog->value()==ProgramMode::HOLD || term_prog->value()==ProgramMode::MANU ? ILI9341_GREENYELLOW : ILI9341_DARKGREY;
    });

    addElement(&DigitalDisplay24pt7b, 1, 72, 100, [](char* str, int* cc){
      int time = term_time->value();
      sprintf(str, time>0?"%02d:%02d":"--:--", (int)floor(time/100), (int)floor(time%100));
      *cc = ILI9341_LIGHTGREY;
    });

    addElement(&DigitalDisplay60pt7b, 1, 70, 220, [](char* str, int* cc){ 
      float t = term_temp->value();
      sprintf(str, "%d", (int)floor(t/10)); 
      *cc = ILI9341_WHITE;
    });
    addElement(&DigitalDisplay60pt7b, 1, 124, 220, [](char* str, int* cc){ 
      float t = term_temp->value();
      sprintf(str, "%d", (int)floor((int)floor(t)%10)); 
      *cc = ILI9341_WHITE;
    });
    addElement(&DigitalDisplay24pt7b, 1, 176, 220, [](char* str, int* cc){ 
      float t = term_temp->value();
      sprintf(str, "%d", (int)floor((t-floor(t))*10)); 
      *cc = ILI9341_WHITE;
    });

    addElement(&DigitalDisplay24pt7b, 1, 100, 290, [](char* str, int* cc){ 
      float t = term_targ->value();
      sprintf(str, "%d", (int)floor(t/10)); 
      *cc = ILI9341_LIGHTGREY;
    });
    addElement(&DigitalDisplay24pt7b, 1, 120, 290, [](char* str, int* cc){ 
      float t = term_targ->value();
      sprintf(str, "%d", (int)floor((int)floor(t)%10)); 
      *cc = ILI9341_LIGHTGREY;
    });
    addElement(&DigitalDisplay12pt7b, 1, 142, 290, [](char* str, int* cc){ 
      float t = term_targ->value();
      sprintf(str, "%d", (int)floor((t-floor(t))*10)); 
      *cc = ILI9341_LIGHTGREY;
    });
    addElement(&ticons12pt7b, 1, 36, 290, [](char* str, int* cc){
      sprintf(str, "%s", "e");
      *cc = ILI9341_LIGHTGREY;
    });
    addElement(&ticons12pt7b, 1, 180, 290, [](char* str, int* cc){
      sprintf(str, "%s", "d");
      *cc = ILI9341_LIGHTGREY;
    });
  }

	int click(TS_Point *p) override {
    if (p->y < 60) {
      if (p->x < 120) {
        term_mode->value() = floor((term_mode->value()+1)%2);
      } else {
        term_prog->value() = floor((term_prog->value()+1)%3);
      }
    } else if (p->y >= 60 && p->y <= 120) {
      return DisplayPage::CLOCK;
    } else if (p->y > 240) {
      if (p->x < 120) {
        term_targ->value() = constrain(term_targ->value() - 0.5, TEMP_MIN, TEMP_MAX);
      } else {
        term_targ->value() = constrain(term_targ->value() + 0.5, TEMP_MIN, TEMP_MAX);
      }
      term_prog->value() = ProgramMode::MANU;
    } else {
      term_prog->value() = ProgramMode::AUTO;
    }
    return -1;
  }
};

class PageClock : public Page {
public:
  PageClock() : Page() {
    addElement(&DigitalDisplay24pt7b, 1, 72, 100, [](char* str, int* cc){
      int time = term_time->value();
      sprintf(str, time>0?"%02d:%02d":"--:--", (int)floor(time/100), (int)floor(time%100));
      *cc = ILI9341_LIGHTGREY;
    });
  }

	int click(TS_Point *p) override {
    return DisplayPage::TEMP;
  }
};

//*****************************************************************************
class SpiDisplay : public Component {

  Adafruit_ILI9341 tft = Adafruit_ILI9341(D0, D8, -1);
  XPT2046_Touchscreen ts = XPT2046_Touchscreen(D3); 

  PageTemp pageTemp = PageTemp();
  PageClock pageClock = PageClock();
  std::vector<Page*> pages = { &pageTemp, &pageClock };
  Page* page = pages[DisplayPage::TEMP];

  void couple() {
    if (term_temp->value() > 0 && term_targ->value() > 0) {
		  if (term_mode->value() == HEAT) {
		    if (!term_flag->value() && term_temp->value() <= term_targ->value() - TEMP_STEP) {
		      term_flag->value() = true;
		    }
		    if (term_flag->value() && term_temp->value() >= term_targ->value() + TEMP_STEP) {
		      term_flag->value() = false;
		    }
		  } 
		  if (term_mode->value() == COOL) {
		    if (term_flag->value() && term_temp->value() <= term_targ->value() - TEMP_STEP) {
		      term_flag->value() = false;
		    }
		    if (!term_flag->value() && term_temp->value() >= term_targ->value() + TEMP_STEP) {
		      term_flag->value() = true;
		    }
		  }
    }
  }

public:
  SpiDisplay() {
    //
  }

  void setup() override {
    ts.begin();
    ts.setRotation(3);
		
    tft.begin();
		tft.setRotation(0);

    page->show(&tft);
  };

  void loop() override {
		if (ts.touched()) {
      TS_Point p = ts.getPoint();
      do { p = ts.getPoint(); } while (ts.touched());
      //
      uint8_t pid = page->click(trans(p));
      if (pid >= DisplayPage::TEMP && pid <= DisplayPage::CLOCK) {
        page = pages[pid];
        page->show(&tft);
      }
		}
    //
    couple();
    page->update(&tft);
  };

  const TS_Point TS_MIN = TS_Point(220,220,0);
  const TS_Point TS_MAX = TS_Point(3800,3700,0);
  TS_Point* trans(TS_Point p) {
    TS_Point* t = new TS_Point(p);
	  t->x = constrain(t->x,TS_MIN.x,TS_MAX.x);
	  t->y = constrain(t->y,TS_MIN.y,TS_MAX.y);
	  t->x = map(p.y, TS_MIN.y, TS_MAX.y, 0, tft.width());
	  t->y = map(p.x, TS_MIN.x, TS_MAX.x, 0, tft.height());
    return t;
  }

};

