Projekt URWIS (User Request Wizard with Integrated Server) – System Zarządzania Ticketami został 
zaprojektowany jako aplikacja wspierająca zarządzanie zgłoszeniami w systemie helpdeskowym.  
W ramach projektu zaimplementowano architekturę opartą na kontrolerach i serwisach, umożliwiającą 
tworzenie, przeglądanie, filtrowanie oraz automatyczne obsługiwanie zgłoszeń użytkowników.  
System wspiera również uwierzytelnianie, sesje użytkowników, podstawowe logowanie działań oraz 
automatyczne odpowiedzi na bazie słów kluczowych. 

Celem projektu było stworzenie modularnego systemu typu „ticket manager”, który: 
  • umożliwia autoryzację i zarządzanie sesjami użytkowników (różne role), 
  • wspiera rejestrację i obsługę zgłoszeń (ticketów), 
  • automatyzuje odpowiedzi na podstawie słów kluczowych, 
  • rejestruje działania systemowe w prostym logu, 
  • umożliwia administratorom zarządzanie słownikami oraz zgłoszeniami, 
  • stanowi przykład poprawnie zaprojektowanego obiektowo systemu w C++. 
  
Do realizacji warstwy interfejsu HTTP wykorzystano bibliotekę Crow – nowoczesny, lekki framework  
dla C++ inspirowany bibliotekami takimi jak Flask (Python) czy Express (Node.js). Crow umożliwia szybkie 
budowanie REST-owych (Representational State Transfer) API, wspiera routing, obsługę żądań, odpowiedzi 
oraz integrację z danymi JSON. Dzięki zastosowaniu tej biblioteki możliwe było stworzenie kompletnego 
backendu wyłącznie w języku C++, bez potrzeby integrowania go z innymi technologiami, takimi jak Python 
czy Node.js.
