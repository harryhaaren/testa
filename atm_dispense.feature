Feature:
  Scenario Outline:
    Given user has a valid card;
    And account balance is <40>;
    When they insert the card;
    And withdraw <17>;
    Then the ATM should return <17>;
    And the balance should be <23>

    Examples:
      | balance_start | withdraw | dispense | balance_end |
      |     50        |    20    |    20    |     30      |

  Scenario Outline:
    Given user has a valid card;
    And account balance is <-7>;
    When they insert the card;
    And withdraw <20>;
    Then the ATM should return <0>;
    And the balance should be <-7>
